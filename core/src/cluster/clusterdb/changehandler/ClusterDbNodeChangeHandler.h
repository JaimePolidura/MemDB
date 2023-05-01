#pragma once

#include "cluster/clusterdb/ClusterDbValueChanged.h"
#include "cluster/Node.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "logging/Logger.h"
#include "cluster/Cluster.h"

class ClusterDbNodeChangeHandler {
protected:
    logger_t logger;

public:
    ClusterDbNodeChangeHandler(logger_t logger): logger(logger) {}

    ClusterDbNodeChangeHandler() = default;

    virtual void handleChange(cluster_t cluster, node_t nodeChanged, const ClusterDbChangeType changeType) = 0;
};

using clusterDbNodeChangeHandler_t = std::shared_ptr<ClusterDbNodeChangeHandler>;