#include <gtest/gtest.h>

#include "cluster/othernodes/ClusterNodes.h"

//See in _deployment partitions run-simple-cluster.sh
TEST(ClusterNodes, setOtherNodesTest) {
    ClusterNodes clusterNodes{};

    auto node1 = std::make_shared<Node>(1, "192.168.1.1", NodeState::RUNNING);
    auto node2 = std::make_shared<Node>(4, "192.168.1.2", NodeState::RUNNING);

    clusterNodes.setOtherNodes({node1, node2}, NodePartitionOptions{.partitionId = 0});

    std::vector<NodesInPartition> groups = clusterNodes.getNodesInPartitions();
    std::set<memdbNodeId_t> group = groups.at(0).getAll();

    ASSERT_TRUE(group.contains(node1->nodeId));
    ASSERT_TRUE(group.contains(node2->nodeId));

    ASSERT_EQ(clusterNodes.getByNodeId(1), node1);
    ASSERT_EQ(clusterNodes.getByNodeId(4), node2);
}