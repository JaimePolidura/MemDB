#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"

PartitionClusterNodeChangeHandler::PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog,
                                                                     operatorDispatcher_t operatorDispatcher):
    ClusterDbNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher),
    newNodePartitionChangeHandler(logger, cluster, operationLog, operatorDispatcher),
    deletionNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher) {}

void PartitionClusterNodeChangeHandler::handleNewNode(node_t newNode) {
    this->newNodePartitionChangeHandler.handle(newNode);
}

void PartitionClusterNodeChangeHandler::handleDeletionNode(node_t deletedNode) {
    this->deletionNodeChangeHandler.handle(deletedNode);
}