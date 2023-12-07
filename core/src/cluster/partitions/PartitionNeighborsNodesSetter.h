#pragma once

#include "cluster/Cluster.h"

class PartitionNeighborsNodesSetter {
private:
    cluster_t cluster;

public:
    PartitionNeighborsNodesSetter(cluster_t cluster);

    PartitionNeighborsNodesSetter() = default;

    void setClusterObject(cluster_t cluster);

    void updateNeighborsWithNewNode(node_t newNode);

    void updateNeighborsWithDeletedNode(node_t deletedNode);

    void addAllNeighborsInPartitions(const GetClusterConfigResponse& response);

private:
    std::vector<RingEntry> getRingEntriesPartitionExceptSelf(RingEntry actualEntry);

    std::vector<node_t> toNodesFromRingEntries(const std::vector<RingEntry>& ringEntries, const std::vector<node_t>& nodes);

    bool containsNodeId(const std::vector<RingEntry>& entries, memdbNodeId_t nodeId);

    memdbNodeId_t getOldNodeIdPartitionMember(RingEntry headPartitionNode);

    memdbNodeId_t getOldNodeIdPartitionMember(const std::vector<RingEntry>& actualNeighbors);
};