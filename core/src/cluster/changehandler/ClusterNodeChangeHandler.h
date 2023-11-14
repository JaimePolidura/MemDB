#pragma once

#include "cluster/Node.h"
#include "logging/Logger.h"
#include "operators/OperatorDispatcher.h"

class Cluster;
using cluster_t = std::shared_ptr<Cluster>;

class ClusterNodeChangeHandler {
protected:
    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

public:
    ClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
        logger(logger), cluster(cluster), operationLog(operationLog), operatorDispatcher(operatorDispatcher) {}

    ClusterNodeChangeHandler() = default;

    virtual void handleNewNode(node_t newNode) = 0;

    virtual void handleDeletionNode(node_t deletedNode) = 0;
};

using clusterNodeChangeHandler_t = std::shared_ptr<ClusterNodeChangeHandler>;