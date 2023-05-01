#pragma once

#include "cluster/clusterdb/ClusterDbValueChanged.h"
#include "cluster/Node.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "logging/Logger.h"
#include "cluster/Cluster.h"

class ClusterDbNodeChangeHandler {
protected:
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

public:
    ClusterDbNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog): logger(logger),
        cluster(cluster), operationLog(operationLog) {}

    ClusterDbNodeChangeHandler() = default;

    virtual void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) = 0;
};

using clusterDbNodeChangeHandler_t = std::shared_ptr<ClusterDbNodeChangeHandler>;