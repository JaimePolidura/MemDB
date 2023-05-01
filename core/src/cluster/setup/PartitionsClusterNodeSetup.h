#pragma once

#include "cluster/clusterdb/changehandler/PartitionClusterNodeChangeHandler.h"
#include "cluster/setup/ClusterNodeSetup.h"

class PartitionsClusterNodeSetup : public ClusterNodeSetup {
public:
    PartitionsClusterNodeSetup(logger_t logger, configuration_t configuration): ClusterNodeSetup(logger, configuration) {}

    void setClusterInformation(cluster_t cluster) override {
        GetRingNeighborsResponse neighborsNodes = cluster->clusterManager->getRingNeighbors(configuration->get(ConfigurationKeys::MEMDB_CORE_NODE_ID));
        GetRingInfoResponse ringInfo = cluster->clusterManager->getRingInfo();

        setOtherNodes(cluster, neighborsNodes.neighbors); //Includes self

        cluster->partitions = std::make_shared<Partitions>(ringInfo.entries, ringInfo.nodesPerPartition, ringInfo.maxSize, configuration);
    }

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog) override {
        return std::make_shared<PartitionClusterNodeChangeHandler>(cluster->logger, cluster, operationLog);
    }
};