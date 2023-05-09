#pragma once

#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/changehandler/simple/SimpleClusterNodeChangeHandler.h"

class SimpleClusterNodeSetup : public ClusterNodeSetup {
public:
    SimpleClusterNodeSetup(logger_t logger, configuration_t configuration): ClusterNodeSetup(logger, configuration) {}

    void setClusterInformation(cluster_t cluster) override {
        memdbNodeId_t selfNodeId = configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        std::vector<node_t> allNodes = cluster->clusterManager->getAllNodes(selfNodeId).nodes;

        setSelfNodeFromAllNodes(cluster, allNodes);
        cluster->clusterNodes->setOtherNodes(getOtherNodesFromAllNodes(allNodes));
    }

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override {
        return std::make_shared<SimpleClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
    }
};