#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"

PartitionClusterNodeChangeHandler::PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog,
                                                                     operatorDispatcher_t operatorDispatcher):
    ClusterDbNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher),
    newNodePartitionChangeHandler(logger, cluster, operationLog, operatorDispatcher),
    deletionNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher) {}

void PartitionClusterNodeChangeHandler::handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) {
    if(changeType == ClusterDbChangeType::DELETED) {
        this->logger->debugInfo("Detected deletion of node {0} in the cluster", nodeChanged->nodeId);
        this->deletionNodeChangeHandler.handle(nodeChanged);
    }else if(cluster->clusterNodes->existsByNodeId(nodeChanged->nodeId)) {
        cluster->clusterNodes->setNodeState(nodeChanged->nodeId, nodeChanged->state);
    }else {
        this->newNodePartitionChangeHandler.handle(nodeChanged);
    }
}