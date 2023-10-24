#include "cluster/changehandler/partition/DeletionNodeChangeHandler.h"

DeletionNodeChangeHandler::DeletionNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
    logger(logger),
    cluster(cluster),
    operationLog(operationLog),
    operatorDispatcher(operatorDispatcher),
    moveOpLogRequestCreator(cluster->configuration->get(ConfigurationKeys::AUTH_NODE_KEY)),
    partitionNeighborsNodesGroupSetter(cluster) {}

void DeletionNodeChangeHandler::handle(node_t deletedNode) {
    this->logger->debugInfo("Detected deletion of node {0}", deletedNode->nodeId);

    if(this->cluster->clusterNodes->existsByNodeId(deletedNode->nodeId)) {
        this->partitionNeighborsNodesGroupSetter.updateNeighborsWithDeletedNode(deletedNode);
    }

    if(this->cluster->selfNode->nodeId == deletedNode->nodeId) {
        std::vector<RingEntry> neighborsClockWise = this->cluster->partitions->getNeighborsClockwise();
        memdbNodeId_t prevNodeId = this->cluster->partitions->getNeighborCounterClockwiseByNodeId(deletedNode->nodeId).nodeId;

        this->sendSelfOplogToPrevNode(prevNodeId);
        this->sendRestOplogsToNextNodes(neighborsClockWise);
    }
}

void DeletionNodeChangeHandler::sendRestOplogsToNextNodes(const std::vector<RingEntry>& neighborsClockWise) {
    uint32_t nodesPerPartition = this->cluster->partitions->getNodesPerPartition();

    for (uint32_t actualOplogId = 1; actualOplogId < nodesPerPartition; actualOplogId++) {
        int affectedNodes = nodesPerPartition - actualOplogId;

        auto bucketIterator = this->cluster->memDbStores->getByPartitionId(actualOplogId)->bucketIterator();

        while(bucketIterator.hasNext()){
            auto actualOplog = bucketIterator.next();

            for (int i = 0; i < affectedNodes; i++) {
                int newOplogId = i + actualOplogId;
                int oldOplog = newOplogId + 1;
                bool nodeAlreadyHoldsOplog = oldOplog < nodesPerPartition;

                memdbNodeId_t nodeIdToSendRequest = neighborsClockWise.at(i).nodeId;

                bool applyNewOplog = !nodeAlreadyHoldsOplog;
                bool clearOldOplog = nodeAlreadyHoldsOplog;

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

    while(bucketIterator.hasNext()) {
        auto selfOplog = bucketIterator.next();

        this->cluster->clusterNodes->sendRequest(prevNodeId, this->moveOpLogRequestCreator.create(CreateMoveOplogReqParams{
            .oplog = selfOplog,
            .applyNewOplog = true,
            .clearOldOplog = false,
            .oldOplogId = 0,
            .newOplogId = 0
        }));
    }
}