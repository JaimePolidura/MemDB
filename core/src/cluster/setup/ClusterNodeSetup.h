#pragma once

#include "cluster/Cluster.h"
#include "cluster/clusterdb/changehandler/ClusterDbNodeChangeHandler.h"

#include "persistence/oplog/OperationLog.h"

class ClusterNodeSetup {
protected:
    configuration_t configuration;
    logger_t logger;

public:
    ClusterNodeSetup(logger_t logger, configuration_t configuration): logger(logger), configuration(configuration) {}

    void initializeNodeInCluster(cluster_t cluster) {
        this->logger->info("Setting up node in the cluster");

        this->setClusterInformation(cluster);
        cluster->setBooting();

        this->logger->info("Cluster node is now set up");
    }

    std::shared_ptr<Cluster> create() {
        return std::make_shared<Cluster>(this->logger, this->configuration);
    }

    virtual void setClusterInformation(cluster_t cluster) = 0;

    virtual clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) = 0;

protected:
    void setOtherNodes(cluster_t cluster, const std::vector<node_t>& allNodes) {
        memdbNodeId_t selfNodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);

        cluster->selfNode = *std::find_if(allNodes.begin(), allNodes.end(), [selfNodeId](node_t node) -> bool {
            return node->nodeId == selfNodeId;
        });

        std::vector<node_t> otherNodes;
        std::copy_if(allNodes.begin(), allNodes.end(), std::back_inserter(otherNodes), [selfNodeId](node_t node) -> bool{
            return node->nodeId != selfNodeId;
        });

        cluster->clusterNodes->setOtherNodes(otherNodes);

        this->logger->info("Other nodes information is set. Total nodes: {0}", otherNodes.size());
    }
};

using clusterNodeSetup_t = std::shared_ptr<ClusterNodeSetup>;