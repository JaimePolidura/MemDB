#pragma once

#include "replication/clusterdb/ClusterDbValueChanged.h"
#include "replication/Node.h"
#include "replication/othernodes/ClusterNodes.h"
#include "logging/Logger.h"

class ClusterDbNodeChangeHandler {
private:
    clusterNodes_t clusterNodes;
    logger_t logger;

public:
    ClusterDbNodeChangeHandler(clusterNodes_t clusterNodes, logger_t logger):
            clusterNodes(clusterNodes), logger(logger) {}
    
    ClusterDbNodeChangeHandler() = default;

    void handleChange(node_t newNode, const ClusterDbChangeType changeType) {
        if(changeType == ClusterDbChangeType::PUT)
            this->updateNodes(newNode);
        else if (changeType == ClusterDbChangeType::DELETED)
            this->deleteNode(newNode);
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