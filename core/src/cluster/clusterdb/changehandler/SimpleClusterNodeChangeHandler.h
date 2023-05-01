#pragma once

#include "ClusterDbNodeChangeHandler.h"
#include "cluster/Cluster.h"

class SimpleClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    SimpleClusterNodeChangeHandler(logger_t logger): ClusterDbNodeChangeHandler(logger) {}

    void handleChange(cluster_t cluster, node_t nodeChanged, const ClusterDbChangeType changeType) override {
        if(changeType == ClusterDbChangeType::PUT)
            this->updateNodes(nodeChanged, cluster);
        else if (changeType == ClusterDbChangeType::DELETED)
            this->deleteNode(nodeChanged, cluster);
    }

private:
    void updateNodes(node_t node, cluster_t cluster) {
        auto existsByNodeId = cluster->clusterNodes->existsByNodeId(node->nodeId);

        if(existsByNodeId){
            this->logger->debugInfo("Detected change of node {0} with new state {1}", node->nodeId, NodeStates::parseNodeStateToString(node->state));
            cluster->clusterNodes->setNodeState(node->nodeId, node->state);
        }else{
            this->logger->debugInfo("Detected new node {0}", node->nodeId);
            cluster->clusterNodes->addNode(node);
        }
    }

    void deleteNode(node_t node, cluster_t cluster) {
        this->logger->debugInfo("Detected deletion of node {0}", node->nodeId);
        cluster->clusterNodes->deleteNodeById(node->nodeId);
    }
};