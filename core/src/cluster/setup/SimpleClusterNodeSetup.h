#pragma once

#include "cluster/setup/ClusterNodeSetup.h"

class SimpleClusterNodeSetup : public ClusterNodeSetup {
private:
    ClusterDbNodeChangeHandler clusterDbNodeChangeHandler;

public:
    SimpleClusterNodeSetup(logger_t logger, configuration_t configuration): ClusterNodeSetup(logger, configuration) {}

    void setClusterInformation(cluster_t cluster) override {
        std::vector<node_t> otherNodes = cluster->clusterManager->getAllNodes().nodes;

        setOtherNodes(cluster, otherNodes);
    }
};