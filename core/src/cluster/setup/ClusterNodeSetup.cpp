#include "cluster/setup/ClusterNodeSetup.h"

void ClusterNodeSetup::initializeNodeInCluster(cluster_t cluster) {
    this->logger->info("Setting up node in the cluster");

    memdbNodeId_t selfNodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);
    AllNodesResponse allNodes = cluster->clusterManager->getAllNodes(selfNodeId);
    cluster->selfNode = allNodes.getNodeById(selfNodeId);
    std::vector<node_t> otherNodes = allNodes.getAllNodesExcept(selfNodeId);

    this->setClusterInformation(cluster, otherNodes);
    cluster->setBooting();

    this->logger->info("Cluster node is now set up");
}

std::shared_ptr<Cluster> ClusterNodeSetup::create() {
    return std::make_shared<Cluster>(this->logger, this->configuration, this->multipleResponses);
}