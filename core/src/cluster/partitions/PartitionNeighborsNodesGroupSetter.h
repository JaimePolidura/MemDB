#pragma once

#include "cluster/Cluster.h"

class PartitionNeighborsNodesGroupSetter {
private:
    cluster_t cluster;

public:
    PartitionNeighborsNodesGroupSetter(cluster_t cluster);

    PartitionNeighborsNodesGroupSetter() = default;

    void setClusterObject(cluster_t cluster);

    void updateNeighborsWithNewNode(node_t newNode);

    void updateNeighborsWithDeletedNode(node_t deletedNode);

    void addAllNeighborsInPartitions();

private:
    std::vector<RingEntry> getRingEntriesGroupExceptSelf(RingEntry actualEntry);

    std::vector<node_t> toNodesFromRingEntries(const std::vector<RingEntry>& ringEntries);

    bool containsNodeId(const std::vector<RingEntry>& entries, memdbNodeId_t nodeId);

    memdbNodeId_t getOldNodeIdPartitionMember(RingEntry headPartitionNode);

    memdbNodeId_t getOldNodeIdPartitionMember(const std::vector<RingEntry>& actualNeighbors);
};