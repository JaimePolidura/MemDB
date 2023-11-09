#include "cluster/setup/ClusterNodeSetup.h"

void ClusterNodeSetup::initializeNodeInCluster() {
    this->logger->info("Setting up node in the cluster");

    cluster->selfNode = clusterDb->getByNodeId(configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID));

    this->setClusterConfig(cluster->getClusterConfig()
            .get_or_throw("Cannot contact to any seed node"));

    cluster->clusterChangeHandler = this->getClusterDbChangeNodeHandler();

    cluster->announceJoin();

    cluster->setBooting();

    this->logger->info("Cluster node is now set up");
}