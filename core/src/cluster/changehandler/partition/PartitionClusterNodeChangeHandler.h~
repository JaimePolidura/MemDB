#pragma once

#include "cluster/changehandler/partition/NewNodePartitionChangeHandler.h"
#include "cluster/changehandler/partition/DeletionNodeChangeHandler.h"
#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher);

    void handleNewNode(node_t newNode) override;

    void handleDeletionNode(node_t deletedNode) override;
private:
    NewNodePartitionChangeHandler newNodePartitionChangeHandler;
    DeletionNodeChangeHandler deletionNodeChangeHandler;
};