#pragma once

#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/simple/SimpleClusterNodeChangeHandler.h"

class SimpleClusterNodeSetup : public ClusterNodeSetup {
public:
    SimpleClusterNodeSetup(logger_t logger, configuration_t configuration): ClusterNodeSetup(logger, configuration) {}

    void setClusterInformation(cluster_t cluster, const std::vector<node_t>& otherNodes) override {
        cluster->clusterNodes->setOtherNodes(otherNodes, NodeGroupOptions{.nodeGroupId = 0});
    }

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override {
        return std::make_shared<SimpleClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
    }
};