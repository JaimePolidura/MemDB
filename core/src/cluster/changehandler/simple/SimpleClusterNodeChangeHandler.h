#pragma once

#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"
#include "cluster/Cluster.h"

class SimpleClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    SimpleClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
        ClusterDbNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher) {}

    void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) override;

private:
    void updateNodes(node_t node, cluster_t cluster);

    void deleteNode(node_t node, cluster_t cluster);
};