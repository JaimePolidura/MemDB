#include "cluster/partitions/PartitionNeighborsNodesSetter.h"

PartitionNeighborsNodesSetter::PartitionNeighborsNodesSetter(cluster_t cluster): cluster(cluster) {}

void PartitionNeighborsNodesSetter::setClusterObject(cluster_t clusterParam) {
    this->cluster = clusterParam;
}

void PartitionNeighborsNodesSetter::updateNeighborsWithDeletedNode(node_t deletedNode) {
    cluster->clusterNodes->deleteNodeById(deletedNode->nodeId);
    cluster->partitions->deleteByNodeId(deletedNode->nodeId);
}

void PartitionNeighborsNodesSetter::updateNeighborsWithNewNode(node_t newNode) {
    uint32_t nodesPerPartition = cluster->getNodesPerPartition();
    RingEntry actualHeadPartition = cluster->partitions->getSelfEntry();

    for (int actualPartitionId = 0; actualPartitionId < nodesPerPartition; ++actualPartitionId) {
        std::vector<RingEntry> neighborsActualPartition = cluster->partitions->getNeighborsClockwiseByNodeId(actualHeadPartition.nodeId);

        if(this->containsNodeId(neighborsActualPartition, newNode->nodeId)){
            cluster->clusterNodes->addNode(newNode, {.partitionId = actualPartitionId});

            memdbNodeId_t oldMemberPartitionNodeId = this->getOldNodeIdPartitionMember(neighborsActualPartition);
            cluster->clusterNodes->removeNodeFromPartition(oldMemberPartitionNodeId, {.partitionId = actualPartitionId});
        }

        actualHeadPartition = cluster->partitions->getNeighborCounterClockwiseByNodeId(actualHeadPartition.nodeId);
    }
}

void PartitionNeighborsNodesSetter::addAllNeighborsInPartitions() {
    uint32_t nodesPerPartition = cluster->partitions->getNodesPerPartition();
    RingEntry actualEntry = cluster->partitions->getSelfEntry();

    for (int i = 0; i < nodesPerPartition; ++i) {
        std::vector<RingEntry> ringEntriesActualPartition = this->getRingEntriesPartitionExceptSelf(actualEntry);
        std::vector<node_t> nodesActualPartition = this->toNodesFromRingEntries(ringEntriesActualPartition);

        cluster->clusterNodes->setOtherNodes(nodesActualPartition, {.partitionId = i});

        actualEntry = cluster->partitions->getNeighborCounterClockwiseByNodeId(actualEntry.nodeId);
    }
}

memdbNodeId_t PartitionNeighborsNodesSetter::getOldNodeIdPartitionMember(const std::vector<RingEntry>& nodesInPartition) {
    RingEntry tailPartitionNode = nodesInPartition.at(nodesInPartition.size() - 1);
    RingEntry headPartitionNode = nodesInPartition.at(nodesInPartition.size() - 1);
    int distanceToHeadFromSelf = cluster->partitions->getDistance(headPartitionNode.nodeId);

    return distanceToHeadFromSelf >= 0 ?
           cluster->partitions->getNeighborClockwiseByNodeId(tailPartitionNode.nodeId).nodeId :
           cluster->partitions->getNeighborCounterClockwiseByNodeId(headPartitionNode.nodeId).nodeId;
}

std::vector<RingEntry> PartitionNeighborsNodesSetter::getRingEntriesPartitionExceptSelf(RingEntry actualEntry) {
    std::vector<RingEntry> ringEntries = cluster->partitions->getNeighborsClockwiseByNodeId(actualEntry.nodeId);
    ringEntries.push_back(cluster->partitions->getByNodeId(actualEntry.nodeId));
    std::vector<RingEntry> ringEntriesWithoutSelfNodeId{};
    ringEntriesWithoutSelfNodeId.reserve(ringEntries.size() - 1);
    std::copy_if(ringEntries.begin(),
                 ringEntries.end(),
                 std::back_inserter(ringEntriesWithoutSelfNodeId),
                 [this](RingEntry ringEntry){return this->cluster->getNodeId() != ringEntry.nodeId;});

    return ringEntriesWithoutSelfNodeId;
}

bool PartitionNeighborsNodesSetter::containsNodeId(const std::vector<RingEntry>& entries, memdbNodeId_t nodeId) {
    return std::any_of(entries.begin(), entries.end(), [nodeId](const RingEntry& it){
        return it.nodeId == nodeId;
    });
}

std::vector<node_t> PartitionNeighborsNodesSetter::toNodesFromRingEntries(const std::vector<RingEntry>& ringEntries) {
    std::vector<node_t> nodesToReturn{};
    nodesToReturn.reserve(ringEntries.size());

    for (const RingEntry& entry : ringEntries) {
        nodesToReturn.push_back(cluster->clusterNodes->getByNodeId(entry.nodeId));
    }

    return nodesToReturn;
}