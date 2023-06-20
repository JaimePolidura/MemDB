#pragma once

#include "cluster/Cluster.h"

class ClusterTest {
private:
    cluster_t cluster;

public:
    explicit ClusterTest(cluster_t cluster): cluster(cluster) {}

    clusterNodes_t getClusterNodes() {
        return cluster->clusterNodes;
    }

    void setPartitions(partitions_t partitions) {
        this->cluster->partitions = partitions;
    }

    void setSelfNodeId(node_t node) {
        cluster->selfNode = node;
    }
};