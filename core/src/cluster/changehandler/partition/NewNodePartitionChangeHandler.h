#pragma once

#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"
#include "cluster/Cluster.h"

#include "logging/Logger.h"

#include "operators/OperatorDispatcher.h"

#include "persistence/OperationLogSerializer.h"
#include "persistence/OperationLogUtils.h"
#include "persistence/OperationLogInvalidator.h"

class NewNodePartitionChangeHandler {
private:
    PartitionNeighborsNodesGroupSetter partitionNeighborsNodesGroupSetter;
    OperationLogInvalidator operationLogInvalidator;
    OperationLogSerializer operationLogSerializer;

    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

public:
    NewNodePartitionChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
        logger(logger), cluster(cluster), operationLog(operationLog), operatorDispatcher(operatorDispatcher) {}

    NewNodePartitionChangeHandler() = default;

    void handle(node_t newNode) {
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

private:
    void sendSelfOplogToNodes(node_t newNode) {
        uint32_t distance = cluster->partitions->getDistanceClockwise(newNode->nodeId);
        uint32_t nodesToSendNewOplog = cluster->partitions->getNodesPerPartition() - distance;
        //+1 to get the last node which will contain a copy of the data. We need to delete its copy. In order to do that, we simply send a movePartitionOplogRequest
        //of oplog nodesPerPartition + 1 to delete it.
        std::vector<RingEntry> neighbors = this->cluster->partitions->getNeighborsClockwise(this->cluster->partitions->getNodesPerPartition() + 1);

        std::vector<OperationBody> selfOplog = this->operationLog->getFromDisk({.operationLogId = 0});
        for(int i = 0; i < nodesToSendNewOplog + 1; i++){
            memdbNodeId_t nodeId = neighbors.at(i + nodesToSendNewOplog + 1).nodeId;
            this->cluster->clusterNodes->sendRequest(nodeId, createMovePartitionOplogRequest(i + nodesToSendNewOplog + 1 , selfOplog));
        }
    }

    void recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded) {
        auto[oplogSelfNode, oplogNextNode] = this->splitSelfOplog(newRingEntryAdded);

        //Send oplogNextNode to next node
        if(!oplogNextNode.empty()){
            cluster->clusterNodes->sendRequest(newRingEntryAdded.nodeId,createMovePartitionOplogRequest(0, oplogNextNode));
            invalidateSelfOplogNextNode(oplogNextNode);
        }

        //Send oplogSelfNode to newOplog
        if(!oplogSelfNode.empty()){
            std::vector<RingEntry> neighbors = this->cluster->partitions->getNeighborsClockwise();
            Request moveOplogRequest = this->createMovePartitionOplogRequest(0, oplogSelfNode);

            for(int i = 0; i < neighbors.size(); i++){
                moveOplogRequest.operation.setArg(0, SimpleString<memDbDataLength_t>::fromNumber(i + 1)); //Set new oplogId

                cluster->clusterNodes->sendRequest(neighbors.at(i).nodeId, moveOplogRequest);
            }
        }
    }

    std::pair<std::vector<OperationBody>, std::vector<OperationBody>> splitSelfOplog(RingEntry newRingEntry) {
        std::vector<OperationBody> allActualOplogs = this->operationLog->getFromDisk(
                OperationLogOptions{.operationLogId = 0});
        std::vector<OperationBody> oplogSelfNode;
        std::vector<OperationBody> oplogNextNode;
        oplogSelfNode.reserve(allActualOplogs.size() / 2);
        oplogNextNode.reserve(allActualOplogs.size() / 2);

        for(const OperationBody& oplog : allActualOplogs){
            SimpleString<memDbDataLength_t> key = oplog.args->at(0);
            bool keyBelongsToNextNode = this->cluster->partitions->getRingPositionByKey(key) >= newRingEntry.ringPosition;

            if(keyBelongsToNextNode)
                oplogNextNode.push_back(oplog);
            else
                oplogSelfNode.push_back(oplog);
        }

        return std::make_pair(oplogSelfNode, oplogNextNode);
    }

    void invalidateSelfOplogNextNode(const std::vector<OperationBody>& oplogNextNode) {
        std::vector<OperationBody> deleteOperations = this->operationLogInvalidator.getInvalidationOperations(oplogNextNode);

        this->operationLog->addAll(deleteOperations, OperationLogOptions{.dontUseBuffer = true});

        std::shared_ptr<Operator> deleteOperator = this->operatorDispatcher->operatorRegistry->get(0x03);
        this->operatorDispatcher->executeOperations(deleteOperator, deleteOperations, {.onlyExecute = true});
    }

    void updateNeighbors() {
        memdbNodeId_t selfNodeId = this->cluster->selfNode->nodeId;
        std::vector<node_t> allNodes = cluster->clusterManager->getAllNodes(selfNodeId).nodes;

        this->partitionNeighborsNodesGroupSetter.setFromOtherNodes(this->cluster, Utils::filter<node_t>(allNodes, ));


        //TODO Add group nodes
        std::string selfNodeId = cluster->configuration->get(ConfigurationKeys::MEMDB_CORE_NODE_ID);
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
};