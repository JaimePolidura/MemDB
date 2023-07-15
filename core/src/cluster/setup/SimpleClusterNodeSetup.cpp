#include "cluster/setup/SimpleClusterNodeSetup.h"

void SimpleClusterNodeSetup::setClusterInformation(cluster_t cluster, const std::vector<node_t>& otherNodes) {
    cluster->clusterNodes->setOtherNodes(otherNodes, NodeGroupOptions{.nodeGroupId = 0});
}

clusterDbNodeChangeHandler_t SimpleClusterNodeSetup::getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) {
    return std::make_shared<SimpleClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
}
