#pragma once

#include "cluster/changehandler/ClusterNodeChangeHandler.h"
#include "cluster/Cluster.h"

#include "persistence/OperationLog.h"

#include "shared.h"

class ClusterNodeSetup {
public:
    ClusterNodeSetup(operatorDispatcher_t operatorDispatcher, onGoingSyncOplogs_t multipleResponses, configuration_t configuration,
        operationLog_t operationLog, memDbStores_t memDbStores, cluster_t cluster, logger_t logger): operatorDispatcher(operatorDispatcher),
        multipleResponses(multipleResponses), configuration(configuration), operationLog(operationLog), memDbStores(memDbStores),
        cluster(cluster), logger(logger) {}

    void initializeNodeInCluster();

    virtual void setClusterConfig(GetClusterConfigResponse clusterConfig) = 0;

    virtual clusterNodeChangeHandler_t getClusterChangeNodeHandler() = 0;

private:
    bool isSelfTheOnlySeedNode();

    void setConfigurationProvidedClusterConfig();

protected:
    operatorDispatcher_t operatorDispatcher;
    onGoingSyncOplogs_t multipleResponses;
    configuration_t configuration;
    operationLog_t operationLog;
    memDbStores_t memDbStores;
    cluster_t cluster;
    logger_t logger;
};

using clusterNodeSetup_t = std::shared_ptr<ClusterNodeSetup>;