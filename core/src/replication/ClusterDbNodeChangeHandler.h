#pragma once

#include "replication/clusterdb/ClusterDbValueChanged.h"
#include "replication/Node.h"
#include "replication/othernodes/ClusterNodesConnections.h"
#include "logging/Logger.h"

class ClusterDbNodeChangeHandler {
private:
    clusterNodesConnections_t clusterNodeConnections;
    logger_t logger;

public:
    ClusterDbNodeChangeHandler(clusterNodesConnections_t clusterNodeConnections, logger_t logger):
        clusterNodeConnections(clusterNodeConnections), logger(logger) {}
    
    ClusterDbNodeChangeHandler() = default;

    void handleChange(Node& newNode, const ClusterDbChangeType changeType) {
        if(changeType == ClusterDbChangeType::PUT)
            this->updateNodes(newNode);
        else if (changeType == ClusterDbChangeType::DELETED)
            this->deleteNode(newNode);
    }

private:
    void updateNodes(Node &node) {
        auto existsByNodeId = this->clusterNodeConnections->existsByNodeId(node.nodeId);

        if(existsByNodeId){
            this->logger->debugInfo("Detected change of node {0} with new state {1}", node.nodeId, NodeStates::parseNodeStateToString(node.state));
            this->clusterNodeConnections->replaceNode(node);
        }else{
            this->logger->debugInfo("Detected new node {0}", node.nodeId);
            this->clusterNodeConnections->addNode(node);
        }
    }

    void deleteNode(const Node &node) {
        this->logger->debugInfo("Detected deletion of node {0}", node.nodeId);
        this->clusterNodeConnections->deleteNodeById(node.nodeId);
    }
};