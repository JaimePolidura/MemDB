#include "cluster/setup/ClusterNodeSetup.h"

void ClusterNodeSetup::initializeNodeInCluster(cluster_t cluster) {
    this->logger->info("Setting up node in the cluster");

    cluster->selfNode = clusterDb->getByNodeId(configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID));

    this->setCustomClusterInformation(cluster);

    cluster->setBooting();

    this->logger->info("Cluster node is now set up");
}

std::shared_ptr<Cluster> ClusterNodeSetup::create() {
    return std::make_shared<Cluster>(this->logger, this->configuration, this->multipleResponses, this->memDbStores, this->clusterDb);
}