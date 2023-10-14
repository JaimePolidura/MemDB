#pragma once

#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"
#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"

class PartitionsClusterNodeSetup : public ClusterNodeSetup {
private:
    PartitionNeighborsNodesGroupSetter partitionNeighborsNodesGroupSetter;

public:
    PartitionsClusterNodeSetup(logger_t logger, configuration_t configuration, onGoingMultipleResponsesStore_t multipleResponses, memDbStores_t memDbStores):
            ClusterNodeSetup(logger, configuration, multipleResponses, memDbStores) {}

    void setCustomClusterInformation(cluster_t cluster) override;

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override;

private:
    std::vector<node_t> neighborsRingEntriesToNodes(const std::vector<RingEntry>& entries);
};