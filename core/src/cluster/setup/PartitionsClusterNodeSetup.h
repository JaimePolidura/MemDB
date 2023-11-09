#pragma once

#include "cluster/partitions/PartitionNeighborsNodesSetter.h"
#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"

class PartitionsClusterNodeSetup : public ClusterNodeSetup {
    PartitionNeighborsNodesSetter partitionNeighborsNodesGroupSetter;

public:
    PartitionsClusterNodeSetup(operatorDispatcher_t operatorDispatcher, onGoingSyncOplogs_t multipleResponses, configuration_t configuration,
            operationLog_t operationLog, memDbStores_t memDbStores, cluster_t cluster, logger_t logger): ClusterNodeSetup(operatorDispatcher,
            multipleResponses, configuration, operationLog, memDbStores, cluster, logger) {}

    void setClusterConfig(GetClusterConfigResponse clusterConfig) override;

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler() override;
};