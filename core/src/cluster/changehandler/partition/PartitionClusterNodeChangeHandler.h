#pragma once

#include "cluster/changehandler/partition/NewNodePartitionChangeHandler.h"
#include "cluster/changehandler/partition/DeletionNodeChangeHandler.h"
#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
private:
    NewNodePartitionChangeHandler newNodePartitionChangeHandler;
    DeletionNodeChangeHandler deletionNodeChangeHandler;

public:
    PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
            ClusterDbNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher),
            newNodePartitionChangeHandler(logger, cluster, operationLog, operatorDispatcher),
            deletionNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher) {}

    void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) override {
        if(changeType == ClusterDbChangeType::DELETED) {
            this->deletionNodeChangeHandler.handle(nodeChanged);
        }else if(cluster->clusterNodes->existsByNodeId(nodeChanged->nodeId)) {
            cluster->clusterNodes->setNodeState(nodeChanged->nodeId, nodeChanged->state);
        }else {
            this->newNodePartitionChangeHandler.handle(nodeChanged);
        }
    }
};