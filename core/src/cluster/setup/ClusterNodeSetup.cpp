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
    uint32_t nodesPerPartition = configuration->get<uint32_t>(ConfigurationKeys::NODES_PER_PARTITION);
    memdbNodeId_t selfNodeId = configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID);
    uint32_t maxPartitionSize = configuration->get<uint32_t>(ConfigurationKeys::MAX_PARTITION_SIZE);
    uint32_t ringPosition = static_cast<uint32_t>(HashCalculator::calculateMD5(std::to_string(selfNodeId)) % maxPartitionSize);
    
    auto clusterConfig = GetClusterConfigResponse{
        .nodesPerPartition = nodesPerPartition,
        .maxPartitionSize = maxPartitionSize,
        .nodes = {},
        .ringEntries = {RingEntry{
            .nodeId = selfNodeId,
            .ringPosition =  ringPosition
        }}
    };

    this->setClusterConfig(clusterConfig);
    this->cluster->persistClusterConfig(clusterConfig);
}

bool ClusterNodeSetup::isSelfTheOnlySeedNode() {
    std::vector<std::string> seedsNodes = configuration->getVector(ConfigurationKeys::SEED_NODES);
    return seedsNodes.size() == 1 && seedsNodes[0].compare(configuration->get(ConfigurationKeys::ADDRESS) + ":" +
    configuration->get(ConfigurationKeys::SERVER_PORT)) == 0;
}