#pragma once

#include "cluster/Cluster.h"
#include "cluster/changehandler/ClusterDbNodeChangeHandler.h"
#include "persistence/oplog/OperationLog.h"

#include "utils/Utils.h"

class ClusterNodeSetup {
protected:
    configuration_t configuration;
    logger_t logger;

public:
    ClusterNodeSetup(logger_t logger, configuration_t configuration): logger(logger), configuration(configuration) {}

    void initializeNodeInCluster(cluster_t cluster) {
        this->logger->info("Setting up node in the cluster");

        memdbNodeId_t selfNodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        AllNodesResponse allNodes = cluster->clusterManager->getAllNodes(selfNodeId);
        cluster->selfNode = allNodes.getNodeById(selfNodeId);
        std::vector<node_t> otherNodes = allNodes.getAllNodesExcept(selfNodeId);

        this->setClusterInformation(cluster, otherNodes);
        cluster->setBooting();

        this->logger->info("Cluster node is now set up");
    }

    std::shared_ptr<Cluster> create() {
        return std::make_shared<Cluster>(this->logger, this->configuration);
    }

    virtual void setClusterInformation(cluster_t cluster, const std::vector<node_t>& otherNodes) = 0;

    virtual clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) = 0;
};

using clusterNodeSetup_t = std::shared_ptr<ClusterNodeSetup>;