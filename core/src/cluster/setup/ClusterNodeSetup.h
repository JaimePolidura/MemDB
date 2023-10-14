#pragma once

#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"
#include "cluster/clusterdb/ClusterDb.h"
#include "cluster/Cluster.h"

#include "persistence/OperationLog.h"

#include "utils/Utils.h"

class ClusterNodeSetup {
protected:
    onGoingMultipleResponsesStore_t multipleResponses;
    configuration_t configuration;
    memDbStores_t memDbStores;
    clusterdb_t clusterDb;
    logger_t logger;

public:
    ClusterNodeSetup(logger_t logger, configuration_t configuration, onGoingMultipleResponsesStore_t multipleResponses, memDbStores_t memDbStores):
        logger(logger), configuration(configuration), multipleResponses(multipleResponses), memDbStores(memDbStores),
        clusterDb(std::make_shared<ClusterDb>(configuration, logger)) {}

    void initializeNodeInCluster(cluster_t cluster);

    std::shared_ptr<Cluster> create();

    virtual void setCustomClusterInformation(cluster_t cluster) = 0;

    virtual clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) = 0;
};

using clusterNodeSetup_t = std::shared_ptr<ClusterNodeSetup>;