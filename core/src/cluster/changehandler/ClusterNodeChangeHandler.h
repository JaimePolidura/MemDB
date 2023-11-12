#pragma once

#include "cluster/Node.h"
#include "logging/Logger.h"
#include "operators/OperatorDispatcher.h"

class Cluster;
using cluster_t = std::shared_ptr<Cluster>;

class ClusterDbNodeChangeHandler {
protected:
    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

public:
    ClusterDbNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
        logger(logger), cluster(cluster), operationLog(operationLog), operatorDispatcher(operatorDispatcher) {}

    ClusterDbNodeChangeHandler() = default;

    virtual void handleNewNode(node_t newNode) = 0;

    virtual void handleDeletionNode(node_t deletedNode) = 0;
};

using clusterDbNodeChangeHandler_t = std::shared_ptr<ClusterDbNodeChangeHandler>;