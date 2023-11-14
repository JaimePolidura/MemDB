#pragma once

#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/simple/SimpleClusterNodeChangeHandler.h"

class SimpleClusterNodeSetup : public ClusterNodeSetup {
public:
    SimpleClusterNodeSetup(operatorDispatcher_t operatorDispatcher, onGoingSyncOplogs_t onGoingSyncOplogs, configuration_t configuration,
            operationLog_t operationLog, memDbStores_t memDbStores, cluster_t cluster, logger_t logger): ClusterNodeSetup(operatorDispatcher,
            onGoingSyncOplogs, configuration, operationLog, memDbStores, cluster, logger) {}

    void setClusterConfig(GetClusterConfigResponse clusterConfig) override;

    clusterNodeChangeHandler_t getClusterChangeNodeHandler() override;
};