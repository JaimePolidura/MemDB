#include "cluster/changehandler/simple/SimpleClusterNodeChangeHandler.h"

void SimpleClusterNodeChangeHandler::handleNewNode(node_t newNode) {
    if(cluster->clusterNodes->existsByNodeId(newNode->nodeId)){
        this->logger->debugInfo("Detected change of node {0}", newNode->nodeId);
        cluster->clusterNodes->addNode(newNode, NodePartitionOptions{.partitionId = 0});
    }else{
        this->logger->debugInfo("Detected new node {0}", newNode->nodeId);
        cluster->clusterNodes->addNode(newNode, NodePartitionOptions{.partitionId = 0});
    }
}

void SimpleClusterNodeChangeHandler::handleDeletionNode(node_t node) {
    this->logger->debugInfo("Detected deletion of node {0}", node->nodeId);
    cluster->clusterNodes->deleteNodeById(node->nodeId);
}