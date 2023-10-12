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

    void setClusterInformation(cluster_t cluster, const std::vector<node_t>& otherNodes) override;

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override;
};