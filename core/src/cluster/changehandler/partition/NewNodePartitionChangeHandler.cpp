#include "cluster/changehandler/partition/NewNodePartitionChangeHandler.h"

NewNodePartitionChangeHandler::NewNodePartitionChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog,
                                                             operatorDispatcher_t operatorDispatcher): logger(logger), cluster(cluster),
                                                             operationLog(operationLog), operatorDispatcher(operatorDispatcher),
                                                             moveOpLogRequestCreator(cluster->configuration->get(ConfigurationKeys::AUTH_NODE_KEY)) {}

void NewNodePartitionChangeHandler::handle(node_t newNode) {
    std::vector<RingEntry> oldClockwiseNeighbors = this->cluster->partitions->getNeighborsClockwise();

    RingEntry ringEntryAdded = cluster->clusterDb->getRingEntryByNodeId(newNode->nodeId);
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
    std::vector<RingEntry> neighbors = this->cluster->partitions->getNeighborsClockwise(this->cluster->partitions->getNodesPerPartition());

    auto bucketIterator = this->cluster->memDbStores->getByPartitionId(0)->bucketIterator();

    while (bucketIterator.hasNext()) {
        std::vector<MapEntry<memDbDataLength_t>> keys = bucketIterator.next();

        //+1 to getAll the last node which will contain a copy of the data. We need to delete its copy. In order to do that, we simply send a movePartitionOplogRequest
        //of oplog nodesPerPartition + 1 to delete it.
        for(int i = 0; i < nodesToSendNewOplog + 1; i++){
            memdbNodeId_t nodeId = neighbors.at(i + nodesToSendNewOplog).nodeId;
            this->cluster->clusterNodes->sendRequest(nodeId, this->moveOpLogRequestCreator.create((CreateMoveOplogReqParams{
                    .oplog = keys,
                    .applyNewOplog = true,
                    .clearOldOplog = true,
                    .newOplogId = static_cast<int>(i + nodesToSendNewOplog + 1)
            })));
        }
    }
}

void NewNodePartitionChangeHandler::recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded, const std::vector<RingEntry>& oldClockwiseNeighbors) {
    auto bucketIterator = this->cluster->memDbStores->getByPartitionId(0)->bucketIterator();

    while(bucketIterator.hasNext()){
        auto[keysStillBeingOwnedByMe, keysOwnedNowByNewNode] = this->splitSelfOplog(bucketIterator.next(), newRingEntryAdded);

        this->removeKeysFromSelfNode(keysOwnedNowByNewNode);
        this->sendNewOplogToNewNode(newRingEntryAdded.nodeId, keysOwnedNowByNewNode);
        this->updateOplogIdOfNeighNodesPlusOne(keysStillBeingOwnedByMe, oldClockwiseNeighbors);
    }
}

void NewNodePartitionChangeHandler::sendNewOplogToNewNode(memdbNodeId_t nodeId, std::vector<MapEntry<memDbDataLength_t>>& oplog) {
    cluster->clusterNodes->sendRequest(nodeId, this->moveOpLogRequestCreator.create(CreateMoveOplogReqParams{
            .oplog = oplog,
            .applyNewOplog = true,
            .clearOldOplog = true,
            .newOplogId = 0
    }));
}

void NewNodePartitionChangeHandler::updateOplogIdOfNeighNodesPlusOne(std::vector<MapEntry<memDbDataLength_t>> &newOplog, const std::vector<RingEntry> &neighbors) {
    Request moveOplogRequest = this->moveOpLogRequestCreator.create(CreateMoveOplogReqParams{
            .oplog = newOplog,
            .applyNewOplog = true,
            .clearOldOplog = true,
            .newOplogId = 0
    });

    for(int i = 0; i < neighbors.size(); i++){
        moveOplogRequest.operation.setArg(0, SimpleString<memDbDataLength_t>::fromNumber(i + 1)); //Set new oplogId
        cluster->clusterNodes->sendRequest(neighbors.at(i).nodeId, moveOplogRequest);
    }
}

void NewNodePartitionChangeHandler::removeKeysFromSelfNode(const std::vector<MapEntry<memDbDataLength_t>>& keysToRemove) {
    std::for_each(keysToRemove.begin(), keysToRemove.end(), [this](const MapEntry<memDbDataLength_t>& keyToRemove) -> void {
        OperationBody deleteOperation = RequestBuilder::builder()
                .operatorNumber(OperatorNumbers::DEL)
                ->addArg(keyToRemove.key)
                ->build()
                .operation;

        this->cluster->memDbStores->getByPartitionId(0)->remove(keyToRemove.key, true, 0, 0);
        this->operationLog->add(deleteOperation, OperationLogOptions{.operationLogId = 0});
    });
}

splitedSelfOplog_t NewNodePartitionChangeHandler::splitSelfOplog(std::vector<MapEntry<memDbDataLength_t>> keysSelfOplog, RingEntry newRingEntry) {
    uint32_t nodePositionNewNode = this->cluster->partitions->getRingPositionByNodeId(newRingEntry.nodeId);
    std::vector<MapEntry<memDbDataLength_t>> keysOwnedByMe{};
    std::vector<MapEntry<memDbDataLength_t>> keysOwnedByOtherNode{};

    for (const MapEntry<memDbDataLength_t>& mapEntry: keysSelfOplog) {
        if(mapEntry.keyHash < nodePositionNewNode) {
            keysOwnedByMe.push_back(mapEntry);
        } else {
            keysOwnedByOtherNode.push_back(mapEntry);
        }
    }

    return std::make_pair(keysOwnedByMe, keysOwnedByOtherNode);
}

void NewNodePartitionChangeHandler::updateNeighbors() {
    this->partitionNeighborsNodesGroupSetter.setFromNewRingEntriesNeighbors(this->cluster, this->cluster->partitions->getNeighbors());
}