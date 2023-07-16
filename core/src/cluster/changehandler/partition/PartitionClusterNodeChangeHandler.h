#pragma once

#include "cluster/changehandler/partition/NewNodePartitionChangeHandler.h"
#include "cluster/changehandler/partition/DeletionNodeChangeHandler.h"
#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
private:
    NewNodePartitionChangeHandler newNodePartitionChangeHandler;
    DeletionNodeChangeHandler deletionNodeChangeHandler;

public:
    PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher);

    void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) override;
};