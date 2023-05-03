#pragma once

#include "cluster/clusterdb/changehandler/ClusterDbNodeChangeHandler.h"
#include "persistence/OperationLogSerializer.h"
#include "persistence/oplog/OperationLog.h"
#include "persistence/OperationLogUtils.h"

class PartitionClusterNodeChangeHandler : public ClusterDbNodeChangeHandler {
private:
    OperationLogSerializer operationLogSerializer;

public:
    PartitionClusterNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
        ClusterDbNodeChangeHandler(logger, cluster, operationLog, operatorDispatcher) {}

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

            cluster->setRunning();
            return;
        }
        if(cluster->partitions->isClockwiseNeighbor(newNode->nodeId)){
            sendSelfOplogToNodes(newNode);

            cluster->setRunning();
            return;
        }
    }

    void sendSelfOplogToNodes(node_t newNode) {
        uint32_t distance = cluster->partitions->getDistanceClockwise(newNode->nodeId);
        uint32_t nodesToSendNewOplog = cluster->partitions->getNodesPerPartition() - distance;
        //+1 to get the last node which will contain a copy of the data. We need to delete its copy. In order to do that, we simply send a movePartitionOplogRequest
        //of oplog nodesPerPartition + 1 to delete it.
        std::vector<RingEntry> neighbors = this->cluster->partitions->getNeighborsClockwise(this->cluster->partitions->getNodesPerPartition() + 1);

        std::vector<OperationBody> selfOplog = this->operationLog->getAllFromDisk({.operationLogId = 0});
        for(int i = 0; i < nodesToSendNewOplog + 1; i++){
            memdbNodeId_t nodeId = neighbors.at(i + nodesToSendNewOplog + 1).nodeId;
            this->cluster->clusterNodes->sendRequest(nodeId, createMovePartitionOplogRequest(i + nodesToSendNewOplog + 1 , selfOplog));
        }
    }

    void recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded) {
        std::vector<OperationBody> allActualOplogs = this->operationLog->getAllFromDisk(OperationLogOptions{.operationLogId = 0});
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

        //Send oplogNextNode to next node
        if(!oplogNextNode.empty()){
            cluster->clusterNodes->sendRequest(newRingEntryAdded.nodeId,
                                               createMovePartitionOplogRequest(0, oplogNextNode));
            invalidateOplogNextNode(oplogNextNode);
        }

        //Send oplogSelfNode to newOplog
        if(!oplogSelfNode.empty()){
            std::vector<RingEntry> neighbors = this->cluster->partitions->getNeighborsClockwise();
            Request moveOplogRequest = this->createMovePartitionOplogRequest(0, oplogSelfNode);

            for(int i = 0; i < neighbors.size(); i++){
                memdbNodeId_t nodeId = neighbors.at(i).nodeId;
                int oplogId = nodeId + 1;

                moveOplogRequest.operation.setArg(0, SimpleString<memDbDataLength_t>::fromNumber(oplogId)); //Set new oplogId

                cluster->clusterNodes->sendRequest(nodeId, moveOplogRequest);
            }
        }

    }

    void invalidateOplogNextNode(const std::vector<OperationBody>& oplogNextNode) {
        std::vector<SimpleString<memDbDataLength_t>> keysInOplogNextNode = OperationLogUtils::getUniqueKeys(oplogNextNode);
        std::vector<OperationBody> deleteOperations(keysInOplogNextNode.size());

        std::transform(keysInOplogNextNode.begin(), keysInOplogNextNode.end(), deleteOperations.begin(),
                       [this](SimpleString<memDbDataLength_t> key ) -> OperationBody{ return this->createDeleteKeyOperation(key);});

        this->operationLog->addAll(deleteOperations, OperationLogOptions{.dontUseBuffer = true});

        std::shared_ptr<Operator> deleteOperator = this->operatorDispatcher->operatorRegistry->get(0x03);
        this->operatorDispatcher->executeOperations(deleteOperator, deleteOperations, {.onlyExecute = true});
    }

    void updateNeighbors() {
        auto selfNodeId = cluster->configuration->get(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        auto neighbors = cluster->clusterManager->getRingNeighbors(selfNodeId).neighbors;
        cluster->clusterNodes->setOtherNodes(neighbors);
    }

    Request createMovePartitionOplogRequest(int newOplogId, const std::vector<OperationBody>& oplog) {
        auto serialized = operationLogSerializer.serializeAllShared(oplog);

        OperationBody operationBody{};
        operationBody.operatorNumber = 0x06; //MovePartitionOplogOperator
        args_t args = OperationBody::createOperationBodyArg();
        args->push_back(SimpleString<memDbDataLength_t>::fromNumber(newOplogId));
        args->push_back(SimpleString<memDbDataLength_t>::fromVector(*serialized));

        Request request{};
        request.operation = operationBody;

        return request;
    }

    OperationBody createDeleteKeyOperation(const SimpleString<memDbDataLength_t>& key) {
        OperationBody operationBody{};
        operationBody.operatorNumber = 0x03; //DeleteOperator
        args_t args = OperationBody::createOperationBodyArg();
        args->push_back(key);

        return operationBody;
    }

    void handleDeletionOfNode(node_t changedNode) {
        //TODO
    }

    void handleChangeStateOfNode(cluster_t cluster, node_t changedNode) {
        cluster->clusterNodes->setNodeState(changedNode->nodeId, changedNode->state);
    }
};