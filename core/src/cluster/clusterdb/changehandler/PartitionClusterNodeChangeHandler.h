#pragma once

#include "cluster/clusterdb/changehandler/ClusterDbNodeChangeHandler.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    PartitionClusterNodeChangeHandler(logger_t logger): ClusterDbNodeChangeHandler(logger) {}

    void handleChange(cluster_t cluster, node_t nodeChanged, const ClusterDbChangeType changeType) override {
        if(changeType == ClusterDbChangeType::DELETED) {
            this->handleDeletionOfNode(cluster, nodeChanged);
        }else if(cluster->clusterNodes->existsByNodeId(nodeChanged->nodeId)) {
            this->handleChangeStateOfNode(cluster, nodeChanged);
        }else {
            this->handleNewNode(cluster, nodeChanged);
        }
    }

private:
    void handleNewNode(cluster_t cluster, node_t newNode) {
        RingEntry ringEntryAdded = cluster->clusterDb->getRingEntryByNodeId(newNode->nodeId);
        if(cluster->selfNode->nodeId == newNode->nodeId)
            return;

        cluster->partitions->add(ringEntryAdded);

        if(!cluster->partitions->isNeighbor(newNode->nodeId))
            return;

        cluster->setBooting();

        auto selfNodeId = cluster->configuration->get(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        auto neighbors = cluster->clusterManager->getRingNeighbors(selfNodeId).neighbors;
        cluster->clusterNodes->setOtherNodes(neighbors);

        //Recompute hash keys of oplog-0 and send to new node
        if(cluster->partitions->getDistanceClockwise(newNode->nodeId) == 1){
            recomputeOplog0(cluster);
        }
        if(cluster->partitions->isCounterClockwiseNeighbor(newNode->nodeId)){
            //-1 oplog of all keys prev to newNode in nodesPerPartition
        }

        cluster->setRunning();
    }

    void recomputeOplog0(cluster_t cluster) {
    }

    void handleDeletionOfNode(cluster_t cluster, node_t changedNode) {
        //TODO
    }

    void handleChangeStateOfNode(cluster_t cluster, node_t changedNode) {
        cluster->clusterNodes->setNodeState(changedNode->nodeId, changedNode->state);
    }
};