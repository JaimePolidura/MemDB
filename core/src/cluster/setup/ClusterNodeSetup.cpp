#include "cluster/setup/ClusterNodeSetup.h"

void ClusterNodeSetup::initializeNodeInCluster() {
    this->logger->info("Setting up node in the cluster. Sending GET_CLUSTER_CONFIG to any node");

    std::result<GetClusterConfigResponse> clusterConfigResponse = cluster->getClusterConfig();

    if(clusterConfigResponse.is_success()) {
        this->setClusterConfig(clusterConfigResponse.get());
    }
    if(clusterConfigResponse.has_error() && !configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)) {
        this->setClusterConfig(GetClusterConfigResponse{.nodes = {}});
    }
    if(clusterConfigResponse.has_error() && configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)) {
        throw std::runtime_error("Cannot connect to any seed node to get cluster information");
    }

    clusterNodeChangeHandler_t changeHandler = this->getClusterChangeNodeHandler();

    cluster->deletedNodeInClusterHandler = {[changeHandler](node_t deletedNode) -> void {
        changeHandler->handleDeletionNode(deletedNode);
    }};

    cluster->newNodeInClusterHandler = {[changeHandler](node_t newNode) -> void {
        changeHandler->handleNewNode(newNode);
    }};

    this->logger->info("Broadcasting to all cluster JOIN_CLUSTER_ANNOUNCE");

    cluster->announceJoin();

    cluster->setBooting();

    this->logger->info("Cluster node is now set up");
}