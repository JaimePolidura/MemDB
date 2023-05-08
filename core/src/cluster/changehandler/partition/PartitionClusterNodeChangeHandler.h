#pragma once

#include "cluster/changehandler/partition/NewNodePartitionChangeHandler.h"
#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
private:
    NewNodePartitionChangeHandler newNodePartitionChangeHandler;

public:
    PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
            ClusterDbNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher),
            newNodePartitionChangeHandler(logger, cluster, operationLog, operatorDispatcher) {}

    void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) override {
        if(changeType == ClusterDbChangeType::DELETED) {
            this->handleDeletionOfNode(nodeChanged);
        }else if(cluster->clusterNodes->existsByNodeId(nodeChanged->nodeId)) {
            this->handleChangeStateOfNode(cluster, nodeChanged);
        }else {
            this->newNodePartitionChangeHandler.handle(nodeChanged);
        }
    }

private:
    void handleDeletionOfNode(node_t changedNode) {
        //TODO
    }

    void handleChangeStateOfNode(cluster_t cluster, node_t changedNode) {
        cluster->clusterNodes->setNodeState(changedNode->nodeId, changedNode->state);
    }
};