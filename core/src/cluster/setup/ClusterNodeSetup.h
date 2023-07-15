#pragma once

#include "cluster/Cluster.h"
#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"
#include "persistence/oplog/OperationLog.h"

#include "utils/Utils.h"

class ClusterNodeSetup {
protected:
    configuration_t configuration;
    logger_t logger;

public:
    ClusterNodeSetup(logger_t logger, configuration_t configuration): logger(logger), configuration(configuration) {}

    void initializeNodeInCluster(cluster_t cluster);

    std::shared_ptr<Cluster> create();

    virtual void setClusterInformation(cluster_t cluster, const std::vector<node_t>& otherNodes) = 0;

    virtual clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) = 0;
};

using clusterNodeSetup_t = std::shared_ptr<ClusterNodeSetup>;