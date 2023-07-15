#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"

void PartitionNeighborsNodesGroupSetter::setFromOtherNodes(cluster_t cluster, const std::vector<node_t>& otherNodes) {
    uint32_t nodesPerPartition = cluster->partitions->getNodesPerPartition();
    RingEntry actualEntry = cluster->partitions->getSelfEntry();

    for (int i = 0; i < nodesPerPartition; ++i) {
        std::vector<RingEntry> ringEntries = getRingEntriesGroupExceptSelf(cluster, actualEntry);
        std::vector<node_t> nodes = this->toNodesFromRingEntries(otherNodes, ringEntries);

        cluster->clusterNodes->setOtherNodes(nodes, NodeGroupOptions {.nodeGroupId = i});

        actualEntry = cluster->partitions->getNeighborCounterClockwiseByNodeId(actualEntry.nodeId);
    }
}

std::vector<RingEntry> PartitionNeighborsNodesGroupSetter::getRingEntriesGroupExceptSelf(cluster_t cluster, RingEntry actualEntry) {
    std::vector<RingEntry> ringEntries = cluster->partitions->getNeighborsClockwiseByNodeId(actualEntry.nodeId);
    ringEntries.push_back(cluster->partitions->getByNodeId(actualEntry.nodeId));
    std::vector<RingEntry> ringEntriesWithoutSelfNodeId{};
    ringEntriesWithoutSelfNodeId.reserve(ringEntries.size() - 1);
    std::copy_if(ringEntries.begin(),
                 ringEntries.end(),
                 std::back_inserter(ringEntriesWithoutSelfNodeId),
                 [cluster](RingEntry ringEntry){return cluster->selfNode->nodeId != ringEntry.nodeId;});

    return ringEntriesWithoutSelfNodeId;
}

std::vector<node_t> PartitionNeighborsNodesGroupSetter::toNodesFromRingEntries(const std::vector<node_t>& allNodes, const std::vector<RingEntry>& ringEntries) {
    std::vector<node_t> nodesToReturn{};
    nodesToReturn.reserve(ringEntries.size());

    for (const RingEntry& entry : ringEntries) {
        nodesToReturn.push_back(getNodeByEntryRing(allNodes, entry));
    }

    return nodesToReturn;
}

node_t PartitionNeighborsNodesGroupSetter::getNodeByEntryRing(const std::vector<node_t>& nodes, RingEntry entry) {
    for (const auto& node: nodes) {
        if(node->nodeId == entry.nodeId){
            return node;
        }
    }

    throw std::runtime_error("Cannot find node when setting up the cluster on PartitionNeighborsNodesGroupSetter");
}