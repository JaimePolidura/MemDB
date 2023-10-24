#include "cluster/changehandler/partition/DeletionNodeChangeHandler.h"

DeletionNodeChangeHandler::DeletionNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
    logger(logger),
    cluster(cluster),
    operationLog(operationLog),
    operatorDispatcher(operatorDispatcher),
    moveOpLogRequestCreator(cluster->configuration->get(ConfigurationKeys::AUTH_NODE_KEY), cluster->getNodeId()),
    partitionNeighborsNodesGroupSetter(cluster) {}

void DeletionNodeChangeHandler::handle(node_t deletedNode) {
    if(this->cluster->clusterNodes->existsByNodeId(deletedNode->nodeId)) {
        this->logger->debugInfo("Detected deletion of node {0}", deletedNode->nodeId);

        this->partitionNeighborsNodesGroupSetter.updateNeighborsWithDeletedNode(deletedNode);
    }

    if(this->cluster->selfNode->nodeId == deletedNode->nodeId) {
        this->logger->debugInfo("Detected self deletion in the cluster", deletedNode->nodeId);

        std::vector<RingEntry> neighborsClockWise = this->cluster->partitions->getNeighborsClockwise();
        memdbNodeId_t prevNodeId = this->cluster->partitions->getNeighborCounterClockwiseByNodeId(deletedNode->nodeId).nodeId;

        this->sendSelfOplogToPrevNode(prevNodeId);
        this->sendRestOplogsToNextNodes(neighborsClockWise);

        exit(-1);
    }
}

void DeletionNodeChangeHandler::sendRestOplogsToNextNodes(const std::vector<RingEntry>& neighborsClockWise) {
    uint32_t nodesPerPartition = this->cluster->partitions->getNodesPerPartition();

    this->logger->debugInfo("Sending rest oplogs to old neighbors clockwise. In total {0} nodes", neighborsClockWise.size());

    for (uint32_t actualOplogId = 1; actualOplogId < nodesPerPartition; actualOplogId++) {
        int affectedNodes = nodesPerPartition - actualOplogId;

        this->logger->debugInfo(" Affected nodes in oplogId {0}: {1}", actualOplogId, affectedNodes);

        auto bucketIterator = this->cluster->memDbStores->getByPartitionId(actualOplogId)->bucketIterator();

        while(bucketIterator.hasNext()){
            auto actualOplog = bucketIterator.next();
            if(actualOplog.empty()){
                continue;
            }

            for (int i = 0; i < affectedNodes; i++) {
                int newOplogId = i + actualOplogId;
                int oldOplog = newOplogId + 1;
                bool nodeAlreadyHoldsOplog = oldOplog < nodesPerPartition;

                memdbNodeId_t nodeIdToSendRequest = neighborsClockWise.at(i).nodeId;

                bool applyNewOplog = !nodeAlreadyHoldsOplog;
                bool clearOldOplog = nodeAlreadyHoldsOplog;

                this->logger->debugInfo(" Sending {0} MOVE_OPLOG(newOplogId = {1}, oldOplogId = {2} applyNewOplog = {3}, clearOldOplog = {4}) Does node already hold that oplog? {5}",
                                        nodeIdToSendRequest, newOplogId, oldOplog, applyNewOplog, clearOldOplog, nodeAlreadyHoldsOplog);

                this->cluster->clusterNodes->sendRequest(nodeIdToSendRequest, this->moveOpLogRequestCreator.create(CreateMoveOplogReqParams{
                    .oplog = actualOplog,
                    .applyNewOplog = applyNewOplog,
                    .clearOldOplog = clearOldOplog,
                    .oldOplogId = oldOplog,
                    .newOplogId = newOplogId
                }));
            }
        }
    }
}

void DeletionNodeChangeHandler::sendSelfOplogToPrevNode(memdbNodeId_t prevNodeId) {
    auto bucketIterator = this->cluster->memDbStores->getByPartitionId(0)->bucketIterator();

    this->logger->debugInfo("Sending self oplog to prev node {0}", prevNodeId);

    while(bucketIterator.hasNext()) {
        auto selfOplog = bucketIterator.next();

        if(selfOplog.empty()){
            continue;
        }

        this->logger->debugInfo(" Sending self oplog bucket to prev node {0} MOVE_OPLOG(newOplogId = {1}, oldOplogId = {2} applyNewOplog = true, clearOldOplog = false)",
                                prevNodeId, 0, 0);

        this->cluster->clusterNodes->sendRequest(prevNodeId, this->moveOpLogRequestCreator.create(CreateMoveOplogReqParams{
            .oplog = selfOplog,
            .applyNewOplog = true,
            .clearOldOplog = false,
            .oldOplogId = 0,
            .newOplogId = 0
        }));
    }
}