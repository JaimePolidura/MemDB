#pragma once

#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/simple/SimpleClusterNodeChangeHandler.h"

class SimpleClusterNodeSetup : public ClusterNodeSetup {
public:
    SimpleClusterNodeSetup(logger_t logger, configuration_t configuration, onGoingMultipleResponsesStore_t multipleResponses, memDbStores_t memDbStores):
        ClusterNodeSetup(logger, configuration, multipleResponses, memDbStores) {}

    void setCustomClusterInformation(cluster_t cluster) override;

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override;
};