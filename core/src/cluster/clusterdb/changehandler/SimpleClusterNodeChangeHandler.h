#pragma once

#include "ClusterDbNodeChangeHandler.h"
#include "cluster/Cluster.h"

class SimpleClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    SimpleClusterNodeChangeHandler(clusterNodes_t clusterNodes, logger_t logger):
            ClusterDbNodeChangeHandler(clusterNodes, logger) {}

    void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) override {
        if(changeType == ClusterDbChangeType::PUT)
            this->updateNodes(nodeChanged);
        else if (changeType == ClusterDbChangeType::DELETED)
            this->deleteNode(nodeChanged);
    }

private:
    void updateNodes(node_t node) {
        auto existsByNodeId = this->clusterNodes->existsByNodeId(node->nodeId);

        if(existsByNodeId){
            this->logger->debugInfo("Detected change of node {0} with new state {1}", node->nodeId, NodeStates::parseNodeStateToString(node->state));
            this->clusterNodes->setNodeState(node->nodeId, node->state);
        }else{
            this->logger->debugInfo("Detected new node {0}", node->nodeId);
            this->clusterNodes->addNode(node);
        }
    }

    void deleteNode(node_t &node) {
        this->logger->debugInfo("Detected deletion of node {0}", node->nodeId);
        this->clusterNodes->deleteNodeById(node->nodeId);
    }
};