#include "cluster/changehandler/partition/NewNodePartitionChangeHandler.h"

NewNodePartitionChangeHandler::NewNodePartitionChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog,
                                                             operatorDispatcher_t operatorDispatcher): logger(logger), cluster(cluster),
                                                             operationLog(operationLog), operatorDispatcher(operatorDispatcher) {}

void NewNodePartitionChangeHandler::handle(node_t newNode) {
    std::vector<RingEntry> oldClockwiseNeighbors = this->cluster->partitions->getNeighborsClockwise();

    RingEntry ringEntryAdded = cluster->clusterDb->getRingEntryByNodeId(newNode->nodeId).entry;
    cluster->partitions->add(ringEntryAdded);

    if(cluster->selfNode->nodeId == newNode->nodeId || !cluster->partitions->isNeighbor(newNode->nodeId))
        return;

    cluster->setBooting();

    updateNeighbors();

    if(cluster->partitions->getDistanceClockwise(newNode->nodeId) == 1){
        recomputeSelfOplogAndSendNextNode(ringEntryAdded, oldClockwiseNeighbors);

        cluster->setRunning();
        return;
    }
    if(cluster->partitions->isClockwiseNeighbor(newNode->nodeId)){
        sendSelfOplogToNodes(newNode);

        cluster->setRunning();
        return;
    }
}

void NewNodePartitionChangeHandler::sendSelfOplogToNodes(node_t newNode) {
    uint32_t distance = cluster->partitions->getDistanceClockwise(newNode->nodeId);
    uint32_t nodesToSendNewOplog = cluster->partitions->getNodesPerPartition() - distance;
    //+1 to get the last node which will contain a copy of the data. We need to delete its copy. In order to do that, we simply send a movePartitionOplogRequest
    //of oplog nodesPerPartition + 1 to delete it.
    std::vector<RingEntry> neighbors = this->cluster->partitions->getNeighborsClockwise(this->cluster->partitions->getNodesPerPartition());
    std::vector<OperationBody> selfOplog = this->operationLog->get({.operationLogId = 0});

    for(int i = 0; i < nodesToSendNewOplog + 1; i++){
        memdbNodeId_t nodeId = neighbors.at(i + nodesToSendNewOplog).nodeId;
        this->cluster->clusterNodes->sendRequest(nodeId, createMovePartitionOplogRequest(i + nodesToSendNewOplog + 1 , selfOplog, false));
    }
}

void NewNodePartitionChangeHandler::recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded, const std::vector<RingEntry>& oldClockwiseNeighbors) {
    auto[oplogSelfNode, oplogNextNode] = this->splitSelfOplog(newRingEntryAdded);

    //Send oplogNextNode to next node
    if(!oplogNextNode.empty()){
        cluster->clusterNodes->sendRequest(newRingEntryAdded.nodeId,createMovePartitionOplogRequest(0, oplogNextNode, true));
        invalidateSelfOplogNextNode(oplogNextNode);
    }

    //Send oplogSelfNode to newOplog
    if(!oplogSelfNode.empty()){
        Request moveOplogRequest = this->createMovePartitionOplogRequest(0, oplogSelfNode, false);

        for(int i = 0; i < oldClockwiseNeighbors.size(); i++){
            moveOplogRequest.operation.setArg(0, SimpleString<memDbDataLength_t>::fromNumber(i + 1)); //Set new oplogId

            cluster->clusterNodes->sendRequest(oldClockwiseNeighbors.at(i).nodeId, moveOplogRequest);
        }
    }
}

std::pair<std::vector<OperationBody>, std::vector<OperationBody>> NewNodePartitionChangeHandler::splitSelfOplog(RingEntry newRingEntry) {
    std::vector<OperationBody> allActualOplogs = this->operationLog->get(
            OperationLogOptions{.operationLogId = 0});
    std::vector<OperationBody> oplogSelfNode;
    std::vector<OperationBody> oplogNextNode;
    oplogSelfNode.reserve(allActualOplogs.size() / 2);
    oplogNextNode.reserve(allActualOplogs.size() / 2);

    for(const OperationBody& oplog : allActualOplogs){
        SimpleString<memDbDataLength_t> key = oplog.args->at(0);
        bool keyBelongsToNextNode = this->cluster->partitions->getRingPositionByKey(key) >= newRingEntry.ringPosition;

        if(keyBelongsToNextNode)
            oplogNextNode.push_back(oplog);
        else
            oplogSelfNode.push_back(oplog);
    }

    return std::make_pair(oplogSelfNode, oplogNextNode);
}

void NewNodePartitionChangeHandler::invalidateSelfOplogNextNode(const std::vector<OperationBody>& oplogNextNode) {
    std::vector<OperationBody> deleteOperations = this->operationLogInvalidator.getInvalidationOperations(oplogNextNode);

    this->operationLog->addAll(deleteOperations, OperationLogOptions{.dontUseBuffer = true});

    std::shared_ptr<Operator> deleteOperator = this->operatorDispatcher->operatorRegistry->get(0x03);
    this->operatorDispatcher->executeOperations(deleteOperator, deleteOperations, {.checkTimestamps = false, .onlyExecute = true});
}

void NewNodePartitionChangeHandler::updateNeighbors() {
    memdbNodeId_t selfNodeId = this->cluster->selfNode->nodeId;
    std::vector<node_t> otherNodes = cluster->clusterManager->getAllNodes(selfNodeId)
            .getAllNodesExcept(selfNodeId);

    this->partitionNeighborsNodesGroupSetter.setFromOtherNodes(this->cluster, otherNodes);
}

Request NewNodePartitionChangeHandler::createMovePartitionOplogRequest(int newOplogId, const std::vector<OperationBody>& oplog, bool applyNewOplog) {
    auto serialized = operationLogSerializer.serializeAllShared(oplog);

    OperationBody operationBody{};
    operationBody.operatorNumber = 0x06; //MovePartitionOplogOperator
    operationBody.flag1 = applyNewOplog; //applyNewOplog
    operationBody.flag2 = true; //clearOldOplog

    args_t args = OperationBody::createOperationBodyArg();
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber(newOplogId));
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber(newOplogId - 1));
    args->push_back(SimpleString<memDbDataLength_t>::fromVector(*serialized));

    Request request{};
    request.operation = operationBody;

    return request;
}