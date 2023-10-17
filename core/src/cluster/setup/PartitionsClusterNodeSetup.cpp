#include "cluster/setup/PartitionsClusterNodeSetup.h"

void PartitionsClusterNodeSetup::setCustomClusterInformation(cluster_t cluster) {
    this->partitionNeighborsNodesGroupSetter.setClusterObject(cluster);

    auto partitionConfiguration = cluster->clusterDb->getPartitionsConfiguration();
    auto ringEntries = cluster->clusterDb->getRingEntries();

    cluster->partitions = std::make_shared<Partitions>(ringEntries, partitionConfiguration.nodesPerPartition, partitionConfiguration.maxSize, configuration);
    cluster->memDbStores = this->memDbStores;
    cluster->clusterNodes->setNumberPartitions(partitionConfiguration.nodesPerPartition);

    this->partitionNeighborsNodesGroupSetter.addAllNeighborsInPartitions();
}

clusterDbNodeChangeHandler_t PartitionsClusterNodeSetup::getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) {
    return std::make_shared<PartitionClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
}

std::vector<node_t> PartitionsClusterNodeSetup::neighborsRingEntriesToNodes(const std::vector<RingEntry>& entries) {
    std::vector<node_t> toReturnNodes{};

    std::for_each(entries.begin(), entries.end(), [&toReturnNodes, this](const RingEntry& entry) {
        toReturnNodes.push_back(this->clusterDb->getByNodeId(entry.nodeId));
    });

    return toReturnNodes;
}