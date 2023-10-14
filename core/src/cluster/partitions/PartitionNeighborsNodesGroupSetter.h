#pragma once

#include "cluster/Cluster.h"

class PartitionNeighborsNodesGroupSetter {
public:
    PartitionNeighborsNodesGroupSetter() = default;

    void setFromNewRingEntriesNeighbors(cluster_t cluster, const std::vector<RingEntry>& otherNodes);

    void setFromOtherNodes(cluster_t cluster, const std::vector<node_t>& otherNodes);

private:
    std::vector<RingEntry> getRingEntriesGroupExceptSelf(cluster_t cluster, RingEntry actualEntry);

    std::vector<node_t> toNodesFromRingEntries(const std::vector<node_t>& allNodes, const std::vector<RingEntry>& ringEntries);

    node_t getNodeByEntryRing(const std::vector<node_t>& nodes, RingEntry entry);
};