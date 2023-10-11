#include "cluster/setup/PartitionsClusterNodeSetup.h"

void PartitionsClusterNodeSetup::setClusterInformation(cluster_t cluster, const std::vector<node_t>& otherNodes) {
    GetRingInfoResponse ringInfo = cluster->clusterManager->getRingInfo();

    cluster->partitions = std::make_shared<Partitions>(ringInfo.entries, ringInfo.nodesPerPartition, ringInfo.maxSize, configuration);
    cluster->memDbStores = this->memDbStores;

    this->partitionNeighborsNodesGroupSetter.setFromOtherNodes(cluster, otherNodes);
}

clusterDbNodeChangeHandler_t PartitionsClusterNodeSetup::getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) {
    return std::make_shared<PartitionClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
}