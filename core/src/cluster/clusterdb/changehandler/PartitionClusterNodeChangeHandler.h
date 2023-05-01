#pragma once

#include "cluster/clusterdb/changehandler/ClusterDbNodeChangeHandler.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
public:
    PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog): ClusterDbNodeChangeHandler(logger, cluster, operationLog) {}

    void handleChange(node_t nodeChanged, const ClusterDbChangeType changeType) override {
        if(changeType == ClusterDbChangeType::DELETED) {
            this->handleDeletionOfNode(nodeChanged);
        }else if(cluster->clusterNodes->existsByNodeId(nodeChanged->nodeId)) {
            this->handleChangeStateOfNode(cluster, nodeChanged);
        }else {
            this->handleNewNode(nodeChanged);
        }
    }

private:
    void handleNewNode(node_t newNode) {
        RingEntry ringEntryAdded = cluster->clusterDb->getRingEntryByNodeId(newNode->nodeId);
        cluster->partitions->add(ringEntryAdded);

        if(cluster->selfNode->nodeId == newNode->nodeId || !cluster->partitions->isNeighbor(newNode->nodeId))
            return;

        cluster->setBooting();

        updateNeighbors();

        if(cluster->partitions->getDistanceClockwise(newNode->nodeId) == 1){
            recomputeSelfOplogAndSendNextNode(ringEntryAdded);
        }
        if(cluster->partitions->isCounterClockwiseNeighbor(newNode->nodeId)){
            //-1 oplog of all keys prev to newNode in nodesPerPartition
        }

        cluster->setRunning();
    }

    void updateNeighbors() {
        auto selfNodeId = cluster->configuration->get(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        auto neighbors = cluster->clusterManager->getRingNeighbors(selfNodeId).neighbors;
        cluster->clusterNodes->setOtherNodes(neighbors);
    }

    void recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded) {
        std::vector<OperationBody> allActualOplogs = this->operationLog->getAllFromDisk(OperationLogQueryOptions{.operationLogId = 0});
        std::vector<OperationBody> oplogSelfNode;
        std::vector<OperationBody> oplogNextNode;
        oplogSelfNode.reserve(allActualOplogs.size() / 2);
        oplogNextNode.reserve(allActualOplogs.size() / 2);

        for(const OperationBody oplog : allActualOplogs){
            SimpleString<memDbDataLength_t> key = oplog.args->at(0);
            bool keyBelongsToNextNode = this->cluster->partitions->getRingPositionByKey(key) >= newRingEntryAdded.ringPosition;

            if(keyBelongsToNextNode)
                oplogNextNode.push_back(oplog);
            else
                oplogSelfNode.push_back(oplog);
        }


    }

    void handleDeletionOfNode(node_t changedNode) {
        //TODO
    }

    void handleChangeStateOfNode(cluster_t cluster, node_t changedNode) {
        cluster->clusterNodes->setNodeState(changedNode->nodeId, changedNode->state);
    }
};