#pragma once

#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/clusterdb/changehandler/SimpleClusterNodeChangeHandler.h"

class SimpleClusterNodeSetup : public ClusterNodeSetup {
public:
    SimpleClusterNodeSetup(logger_t logger, configuration_t configuration): ClusterNodeSetup(logger, configuration) {}

    void setClusterInformation(cluster_t cluster) override {
        std::vector<node_t> otherNodes = cluster->clusterManager->getAllNodes().nodes;

        setOtherNodes(cluster, otherNodes);
    }

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog) override {
        return std::make_shared<SimpleClusterNodeChangeHandler>(cluster->logger, cluster, operationLog);
    }
};