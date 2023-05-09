#pragma once

#include "cluster/changehandler/partition/PartitionClusterNodeChangeHandler.h"
#include "cluster/setup/ClusterNodeSetup.h"

class PartitionsClusterNodeSetup : public ClusterNodeSetup {
public:
    PartitionsClusterNodeSetup(logger_t logger, configuration_t configuration): ClusterNodeSetup(logger, configuration) {}

    void setClusterInformation(cluster_t cluster) override {
        memdbNodeId_t selfNodeId = configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        AllNodesResponse allNeighborsNodes = cluster->clusterManager->getAllNodes(selfNodeId);
        GetRingInfoResponse ringInfo = cluster->clusterManager->getRingInfo();

        cluster->partitions = std::make_shared<Partitions>(ringInfo.entries, ringInfo.nodesPerPartition, ringInfo.maxSize, configuration);

        setSelfNodeFromAllNodes(cluster, allNeighborsNodes.nodes);
        std::vector<node_t> otherNodes = getOtherNodesFromAllNodes(allNeighborsNodes.nodes);
        setOtherToNodesToGroupNodes(otherNodes, ringInfo, cluster);
    }

    clusterDbNodeChangeHandler_t getClusterDbChangeNodeHandler(cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher) override {
        return std::make_shared<PartitionClusterNodeChangeHandler>(cluster->logger, cluster, operationLog, operatorDispatcher);
    }

private:
    void setOtherToNodesToGroupNodes(const std::vector<node_t> otherNodes, GetRingInfoResponse ringInfo, cluster_t cluster) {
        RingEntry actualEntry = cluster->partitions->getSelfEntry();

        for (int i = 0; i < ringInfo.nodesPerPartition; ++i) {
            std::vector<RingEntry> ringEntries = cluster->partitions->getNeighborsClockwiseByNodeId(
                    actualEntry.nodeId, ringInfo.nodesPerPartition - 1); //-1 to avoid including an extra node
            std::vector<node_t> nodes = this->toNodesFromRingEntries(otherNodes, ringEntries);

            cluster->clusterNodes->setOtherNodes(nodes, NodeGroupOptions {.nodeGroupId = i});

            actualEntry = cluster->partitions->getNeighborCounterClockwiseByNodeId(actualEntry.nodeId);
        }
    }

    std::vector<node_t> toNodesFromRingEntries(const std::vector<node_t>& allNodes, const std::vector<RingEntry>& ringEntries) {
        std::vector<node_t> nodesToReturn{ringEntries.size()};

        for (const RingEntry& entry : ringEntries) {
            nodesToReturn.push_back(getNodeByEntryRing(allNodes, entry));
        }

        return nodesToReturn;
    }

    node_t getNodeByEntryRing(const std::vector<node_t>& nodes, RingEntry entry) {
        for (const auto& node: nodes) {
            if(node->nodeId == entry.nodeId){
                return node;
            }
        }

        throw std::runtime_error("Cannot find node when setting up the cluster");
    }
};