#include "gtest/gtest.h"

#include "cluster/partitions/PartitionNeighborsNodesSetter.h"
#include "cluster/Cluster.h"

#include "PartitionsTest.h"
#include "../ClusterTest.h"

TEST(PartitionNeighborsNodesGroupSetter, setFromOtherNodes){
    cluster_t cluster = std::make_shared<Cluster>();
    PartitionTest partitionTest{cluster->getPartitionObject()};
    PartitionNeighborsNodesSetter toTest{};
    ClusterTest clusterTest{cluster};

    clusterTest.setSelfNodeId(std::make_shared<Node>(1, "192.168.1.2", NodeState::RUNNING));
    partitionTest.setNodesPerPartition(2);
    partitionTest.setSelfRingEntry(RingEntry{.nodeId = 1, .ringPosition = 2});
    partitionTest.setRingEntries({
        RingEntry{.nodeId = 1, .ringPosition =  2},
        RingEntry{.nodeId = 4, .ringPosition = 29},
        RingEntry{.nodeId = 2, .ringPosition = 35},
        RingEntry{.nodeId = 3, .ringPosition = 62}
    });

    toTest.setFromOtherNodes(cluster, {
        std::make_shared<Node>(1, "192.168.1.2", NodeState::RUNNING),
        std::make_shared<Node>(4, "192.168.1.3", NodeState::RUNNING),
        std::make_shared<Node>(2, "192.168.1.4", NodeState::RUNNING),
        std::make_shared<Node>(3, "192.168.1.5", NodeState::RUNNING)
    });

    std::vector<NodesInPartition> groups = clusterTest.getClusterNodes()->getNodesInPartitions();

    ASSERT_EQ(groups.size(), 2);

    ASSERT_TRUE(groups.at(0).getAll().contains(4));
    ASSERT_TRUE(groups.at(1).getAll().contains(3));
}