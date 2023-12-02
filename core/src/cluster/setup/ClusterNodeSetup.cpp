#include "cluster/setup/ClusterNodeSetup.h"

void ClusterNodeSetup::initializeNodeInCluster() {
    this->logger->info("Setting up node in the cluster. Sending GET_CLUSTER_CONFIG to any node");

    std::result<GetClusterConfigResponse> clusterConfigResponse = cluster->getClusterConfig();
    bool selfIsTheOnlySeedNode = this->isSelfTheOnlySeedNode();

    if(clusterConfigResponse.is_success()) {
        this->setClusterConfig(clusterConfigResponse.get());
    }
    if(!selfIsTheOnlySeedNode && clusterConfigResponse.has_error()) {
        throw std::runtime_error("Cannot connect to any seed node to get cluster information");
    }
    if(selfIsTheOnlySeedNode && clusterConfigResponse.has_error()) {
        this->setConfigurationProvidedClusterConfig();
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

void ClusterNodeSetup::setConfigurationProvidedClusterConfig() {
    this->setClusterConfig(GetClusterConfigResponse{
        .nodesPerPartition = configuration->get<uint32_t>(ConfigurationKeys::NODES_PER_PARTITION),
        .maxPartitionSize = configuration->get<uint32_t>(ConfigurationKeys::MAX_PARTITION_SIZE),
        .nodes = {},
        .ringEntries = {}
    });
}

bool ClusterNodeSetup::isSelfTheOnlySeedNode() {
    std::vector<std::string> seedsNodes = configuration->getVector(ConfigurationKeys::SEED_NODES);
    return seedsNodes.size() == 1 && seedsNodes[0].compare(configuration->get(ConfigurationKeys::ADDRESS)) == 0;
}