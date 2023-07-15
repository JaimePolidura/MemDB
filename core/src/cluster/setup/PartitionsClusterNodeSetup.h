#pragma once

#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"
#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"
#include "cluster/setup/ClusterNodeSetup.h"

class PartitionsClusterNodeSetup : public ClusterNodeSetup {
private:
    PartitionNeighborsNodesGroupSetter partitionNeighborsNodesGroupSetter;

public:
    PartitionsClusterNodeSetup(logger_t logger, configuration_t configuration): ClusterNodeSetup(logger, configuration) {}

    void setClusterInformation(cluster_t cluster, const std::vector<node_t>& otherNodes) override;

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override;
};