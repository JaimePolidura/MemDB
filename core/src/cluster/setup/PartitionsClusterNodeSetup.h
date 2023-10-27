#pragma once

#include "cluster/partitions/PartitionNeighborsNodesSetter.h"
#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"

class PartitionsClusterNodeSetup : public ClusterNodeSetup {
private:
    PartitionNeighborsNodesSetter partitionNeighborsNodesGroupSetter;

public:
    PartitionsClusterNodeSetup(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t multipleResponses, memDbStores_t memDbStores):
            ClusterNodeSetup(logger, configuration, multipleResponses, memDbStores) {}

    void setCustomClusterInformation(cluster_t cluster) override;

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override;

private:
    std::vector<node_t> neighborsRingEntriesToNodes(const std::vector<RingEntry>& entries);
};