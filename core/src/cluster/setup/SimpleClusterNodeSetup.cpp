#include "cluster/setup/SimpleClusterNodeSetup.h"

void SimpleClusterNodeSetup::setClusterConfig(GetClusterConfigResponse clusterConfig) {
    cluster->partitions = std::make_shared<Partitions>(cluster->configuration);
    cluster->clusterNodes->setNumberPartitions(1);
    cluster->clusterNodes->setOtherNodes(clusterConfig.nodes, NodePartitionOptions{.partitionId = 0});
}

clusterDbNodeChangeHandler_t SimpleClusterNodeSetup::getClusterDbChangeNodeHandler() {
    return std::make_shared<SimpleClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
}
