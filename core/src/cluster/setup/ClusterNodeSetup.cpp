#include "cluster/setup/ClusterNodeSetup.h"

void ClusterNodeSetup::initializeNodeInCluster() {
    this->logger->info("Setting up node in the cluster");

    this->setClusterConfig(cluster->getClusterConfig()
            .get_or_throw("Cannot contact to any seed node"));

    clusterDbNodeChangeHandler_t changeHandler = this->getClusterDbChangeNodeHandler();

    cluster->deletedNodeInClusterHandler = {[changeHandler](node_t deletedNode) -> void {
        changeHandler->handleDeletionNode(deletedNode);
    }};

    cluster->newNodeInClusterHandler = {[changeHandler](node_t newNode) -> void {
        changeHandler->handleNewNode(newNode);
    }};

    cluster->announceJoin();

    cluster->setBooting();

    this->logger->info("Cluster node is now set up");
}