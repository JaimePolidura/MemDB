#include "cluster/changehandler/partition/NewNodePartitionChangeHandler.h"

NewNodePartitionChangeHandler::NewNodePartitionChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog,
                                                             operatorDispatcher_t operatorDispatcher): logger(logger), cluster(cluster),
                                                             partitionNeighborsNodesGroupSetter(cluster), operationLog(operationLog),
                                                             operatorDispatcher(operatorDispatcher),
                                                             moveOpLogRequestCreator(cluster->configuration->get(ConfigurationKeys::AUTH_NODE_KEY), cluster->getNodeId()) {}

void NewNodePartitionChangeHandler::handle(node_t newNode) {
    std::vector<RingEntry> oldClockwiseNeighbors = this->cluster->partitions->getNeighborsClockwise();

    RingEntry ringEntryAdded = cluster->clusterDb->getRingEntryByNodeId(newNode->nodeId);
    cluster->partitions->add(ringEntryAdded);

    if(cluster->selfNode->nodeId == newNode->nodeId || !cluster->partitions->isNeighbor(newNode->nodeId))
        return;

    this->partitionNeighborsNodesGroupSetter.updateNeighborsWithNewNode(newNode);

    if(cluster->partitions->getDistanceClockwise(newNode->nodeId) == 1) {
        cluster->setBooting();

        this->logger->debugInfo("Detected new node {0} next to me in the ring", newNode->nodeId);

        recomputeSelfOplogAndSendNextNode(ringEntryAdded, oldClockwiseNeighbors);

        cluster->setRunning();
        return;
    }
    if(cluster->partitions->isClockwiseNeighbor(newNode->nodeId)){
        cluster->setBooting();

        this->logger->debugInfo("Detected new node {0} clockwise neighbor", newNode->nodeId);

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

        if(keys.empty()){
            continue;
        }

        for(int i = 0; i < nodesToSendNewOplog + 1; i++) {
            auto nodeId = neighbors.at(distance + 1 + i).nodeId;
            auto oldOplog = this->cluster->partitions->getDistance(nodeId) - 1;
            auto newOplog = oldOplog + 1;

            this->logger->debugInfo("Sending self oplog MOVE_OPLOG(newOplogId = {0}, oldOplogId = {1} applyNewOplog = true, clearOldOplog = true) to node {2}",
                                    newOplog, oldOplog, nodeId);

            this->cluster->clusterNodes->sendRequest(nodeId, this->moveOpLogRequestCreator.create((CreateMoveOplogReqParams{
                    .oplog = keys,
                    .applyNewOplog = true,
                    .clearOldOplog = true,
                    .oldOplogId =  oldOplog,
                    .newOplogId = newOplog,
            })));
        }
    }
}

void NewNodePartitionChangeHandler::recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded, const std::vector<RingEntry>& oldClockwiseNeighbors) {
    this->logger->debugInfo("Recomputing self oplog. Starting to iterate map buckets");
    auto bucketIterator = this->cluster->memDbStores->getByPartitionId(0)->bucketIterator();

    while(bucketIterator.hasNext()){
        auto[keysStillBeingOwnedByMe, keysOwnedNowByNewNode] = this->splitSelfOplog(bucketIterator.next(), newRingEntryAdded);

        if(!keysStillBeingOwnedByMe.empty() || !keysOwnedNowByNewNode.empty()){
            this->logger->debugInfo("Recomputed bucket. Keys owned by me {0} keys not owned by new node {1}", keysStillBeingOwnedByMe.size(), keysOwnedNowByNewNode.size());
        }

        this->removeKeysFromSelfNode(keysOwnedNowByNewNode);
        this->sendNewOplogToNewNode(newRingEntryAdded.nodeId, keysOwnedNowByNewNode);
        this->updateOplogIdOfNeighNodesPlusOne(keysStillBeingOwnedByMe, oldClockwiseNeighbors);
    }
}

void NewNodePartitionChangeHandler::sendNewOplogToNewNode(memdbNodeId_t nodeId, std::vector<MapEntry<memDbDataLength_t>>& oplog) {
    if(oplog.empty()){
        return;
    }

    this->logger->debugInfo(" Sending {0} entries from self oplog to node {1} with MOVE_OPLOG(newOplogId = 0, applyNewOplog = true, clearOldOplog = true) ", oplog.size(), nodeId);

    cluster->clusterNodes->sendRequest(nodeId, this->moveOpLogRequestCreator.create(CreateMoveOplogReqParams{
            .oplog = oplog,
            .applyNewOplog = true,
            .clearOldOplog = true,
            .newOplogId = 0
    }));
}

void NewNodePartitionChangeHandler::updateOplogIdOfNeighNodesPlusOne(std::vector<MapEntry<memDbDataLength_t>> &newOplog, const std::vector<RingEntry> &neighbors) {
    if(newOplog.empty()){
        return;
    }

    Request moveOplogRequest = this->moveOpLogRequestCreator.create(CreateMoveOplogReqParams{
            .oplog = newOplog,
            .applyNewOplog = true,
            .clearOldOplog = true,
            .newOplogId = 0
    });

    for(int i = 0; i < neighbors.size(); i++){
        memdbNodeId_t nodeId = neighbors.at(i).nodeId;
        moveOplogRequest.operation.setArg(0, SimpleString<memDbDataLength_t>::fromNumber(i + 1)); //Set new oplogId
        cluster->clusterNodes->sendRequest(nodeId, moveOplogRequest);

        this->logger->debugInfo(" Sending to node {0} MOVE_OPLOG(newOplogId = {1}, applyNewOplog = true, clearOldOplog = true) {2} entries of self oplog",
                                nodeId, i + 1, newOplog.size());
    }
}

void NewNodePartitionChangeHandler::removeKeysFromSelfNode(const std::vector<MapEntry<memDbDataLength_t>>& keysToRemove) {
    if(!keysToRemove.empty()){
        this->logger->debugInfo(" Removing {0} entries from self oplog", keysToRemove.size());
    }

    std::for_each(keysToRemove.begin(), keysToRemove.end(), [this](const MapEntry<memDbDataLength_t>& keyToRemove) -> void {
        OperationBody deleteOperation = RequestBuilder::builder()
                .operatorNumber(OperatorNumbers::DEL)
                ->addArg(keyToRemove.key)
                ->build()
                .operation;

        this->cluster->memDbStores->getByPartitionId(0)->remove(keyToRemove.key, LamportClock{}, LamportClock::UpdateClockStrategy::NONE, nullptr, false);
        this->operationLog->add(0, deleteOperation);
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