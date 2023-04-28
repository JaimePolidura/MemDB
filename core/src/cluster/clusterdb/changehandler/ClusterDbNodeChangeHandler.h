#pragma once

#include "cluster/clusterdb/ClusterDbValueChanged.h"
#include "cluster/Node.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "logging/Logger.h"

class ClusterDbNodeChangeHandler {
protected:
    clusterNodes_t clusterNodes;
    logger_t logger;

public:
    ClusterDbNodeChangeHandler(clusterNodes_t clusterNodes, logger_t logger):
            clusterNodes(clusterNodes), logger(logger) {}

    ClusterDbNodeChangeHandler() = default;

    virtual void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) = 0;
};

using clusterDbNodeChangeHandler_t = std::shared_ptr<ClusterDbNodeChangeHandler>;