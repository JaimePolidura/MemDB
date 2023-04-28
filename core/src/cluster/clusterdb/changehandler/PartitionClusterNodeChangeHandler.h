#pragma once

#include "cluster/clusterdb/changehandler/ClusterDbNodeChangeHandler.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    PartitionClusterNodeChangeHandler(clusterNodes_t clusterNodes, logger_t logger): ClusterDbNodeChangeHandler(clusterNodes, logger) {}

    void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) override {
        //TODO
    }
};