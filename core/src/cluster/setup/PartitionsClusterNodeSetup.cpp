#include "cluster/setup/PartitionsClusterNodeSetup.h"

void PartitionsClusterNodeSetup::setClusterConfig(GetClusterConfigResponse clusterConfig) {
    this->partitionNeighborsNodesGroupSetter.setClusterObject(cluster);

    std::vector<RingEntry> ringEntries = this->getRingEntriesFromClusterConfig(clusterConfig);

    cluster->partitions = std::make_shared<Partitions>(ringEntries, clusterConfig.nodesPerPartition, clusterConfig.maxPartitionSize, configuration);
    cluster->memDbStores = this->memDbStores;
    cluster->clusterNodes->setNumberPartitions(clusterConfig.nodesPerPartition);

    this->partitionNeighborsNodesGroupSetter.addAllNeighborsInPartitions();
}

std::vector<RingEntry> PartitionsClusterNodeSetup::getRingEntriesFromClusterConfig(const GetClusterConfigResponse& clusterConfig) {
    std::vector<RingEntry> ringEntries = clusterConfig.ringEntries;
    memdbNodeId_t selfNodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);

    bool selfNodeIdContainedInClusterConfig = std::any_of(ringEntries.begin(), ringEntries.end(), [this, selfNodeId](RingEntry ringEntry) -> bool {
        return selfNodeId == ringEntry.nodeId;
    });

    if(!selfNodeIdContainedInClusterConfig) {
        uint32_t selfRingPosition = static_cast<uint32_t>(HashCalculator::calculateMD5(std::to_string(selfNodeId))
            % clusterConfig.maxPartitionSize);

        ringEntries.push_back(RingEntry{
            .nodeId = selfNodeId,
            .ringPosition = selfRingPosition
        });
    }

    return ringEntries;
}

clusterNodeChangeHandler_t PartitionsClusterNodeSetup::getClusterChangeNodeHandler() {
    return std::make_shared<PartitionClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
}