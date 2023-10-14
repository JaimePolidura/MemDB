#include "cluster/setup/SimpleClusterNodeSetup.h"

void SimpleClusterNodeSetup::setCustomClusterInformation(cluster_t cluster) {
    auto allNodes = cluster->clusterDb->getAllNodes()
            .getAllNodesExcept(cluster->getNodeId());

    cluster->clusterNodes->setNumberGroups(1);
    cluster->clusterNodes->setOtherNodes(allNodes, NodeGroupOptions{.nodeGroupId = 0});
}

clusterDbNodeChangeHandler_t SimpleClusterNodeSetup::getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) {
    return std::make_shared<SimpleClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
}
