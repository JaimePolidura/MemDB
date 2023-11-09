#include "cluster/setup/PartitionsClusterNodeSetup.h"

void PartitionsClusterNodeSetup::setClusterConfig(GetClusterConfigResponse clusterConfig) {
    this->partitionNeighborsNodesGroupSetter.setClusterObject(cluster);

    cluster->partitions = std::make_shared<Partitions>(clusterConfig.ringEntries, clusterConfig.nodesPerPartition, clusterConfig.maxPartitionSize, configuration);
    cluster->memDbStores = this->memDbStores;
    cluster->clusterNodes->setNumberPartitions(clusterConfig.nodesPerPartition);

    this->partitionNeighborsNodesGroupSetter.addAllNeighborsInPartitions();
}

clusterDbNodeChangeHandler_t PartitionsClusterNodeSetup::getClusterDbChangeNodeHandler() {
    return std::make_shared<PartitionClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
}