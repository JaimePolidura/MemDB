#pragma once

#include "..\ClusterNodeChangeHandler.h"

class SimpleClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    SimpleClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
        ClusterDbNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher) {}

    void handleNewNode(node_t newNode) override;

    void handleDeletionNode(node_t deletedNode) override;
};