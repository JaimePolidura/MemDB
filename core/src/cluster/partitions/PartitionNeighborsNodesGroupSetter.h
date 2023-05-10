#pragma once

#include "cluster/Cluster.h"

class PartitionNeighborsNodesGroupSetter {
public:
    PartitionNeighborsNodesGroupSetter() = default;

    void setFromOtherNodes(cluster_t cluster, const std::vector<node_t>& otherNodes) {
        uint32_t nodesPerPartition = cluster->partitions->getNodesPerPartition();
        RingEntry actualEntry = cluster->partitions->getSelfEntry();

        for (int i = 0; i < nodesPerPartition; ++i) {
            std::vector<RingEntry> ringEntries = cluster->partitions->getNeighborsClockwiseByNodeId(
                    actualEntry.nodeId, nodesPerPartition - 1); //-1 to avoid including an extra node
            std::vector<node_t> nodes = this->toNodesFromRingEntries(otherNodes, ringEntries);

            cluster->clusterNodes->setOtherNodes(nodes, NodeGroupOptions {.nodeGroupId = i});

            actualEntry = cluster->partitions->getNeighborCounterClockwiseByNodeId(actualEntry.nodeId);
        }
    }

private:
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

        throw std::runtime_error("Cannot find node when setting up the cluster on PartitionNeighborsNodesGroupSetter");
    }
};