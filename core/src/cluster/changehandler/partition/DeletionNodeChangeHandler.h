#pragma once

#include "logging/Logger.h"
#include "cluster/Cluster.h"
#include "operators/OperatorDispatcher.h"
#include "persistence/OperationLogSerializer.h"
#include "persistence/OperationLogUtils.h"
#include "persistence/OperationLogInvalidator.h"

class DeletionNodeChangeHandler {
private:
    PartitionNeighborsNodesGroupSetter partitionNeighborsNodesGroupSetter;
    OperationLogSerializer operationLogSerializer;

    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

public:
    DeletionNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher):
    logger(logger), cluster(cluster), operationLog(operationLog), operatorDispatcher(operatorDispatcher) {}

    DeletionNodeChangeHandler() = default;

    void handle(node_t deletedNode) {
        uint32_t distanceClockwise = this->cluster->partitions->getDistanceClockwise(deletedNode->nodeId);
        std::vector<RingEntry> neighborsClockWise = this->cluster->partitions->getNeighborsClockwise();
        memdbNodeId_t prevNodeId = this->cluster->partitions->getNeighborCounterClockwiseByNodeId(deletedNode->nodeId).nodeId;

        this->cluster->partitions->deleteByNodeId(deletedNode->nodeId);
        this->cluster->clusterNodes->deleteNodeById(deletedNode->nodeId);
        this->updateNeighbors();

        if(!this->cluster->partitions->isClockwiseNeighbor(deletedNode->nodeId) && distanceClockwise > 1){
            return;
        }

        cluster->setBooting();

        if(this->cluster->selfNode->nodeId == deletedNode->nodeId){
            this->sendSelfOplogToPrevNode(prevNodeId);
            this->sendRestOplogsToNextNodes(neighborsClockWise);
        }
    }

private:
    void sendRestOplogsToNextNodes(const std::vector<RingEntry>& neighborsClockWise) {
        uint32_t nodesPerPartition = this->cluster->partitions->getNodesPerPartition();

        for (uint32_t actualOplogId = 1; actualOplogId < nodesPerPartition; actualOplogId++) {
            int affectedNodes = nodesPerPartition - actualOplogId;

            std::vector<OperationBody> actualOplog = this->operationLog->getFromDisk(OperationLogOptions{.operationLogId = actualOplogId});

            for (int i = 0; i < affectedNodes; i++) {
                int newOplogId = i + actualOplogId;
                int oldOplog = newOplogId + 1;
                bool nodeAlreadyHoldsOplog = oldOplog < nodesPerPartition;

                memdbNodeId_t nodeIdToSendRequest = neighborsClockWise.at(i).nodeId;

                bool applyNewOplog = !nodeAlreadyHoldsOplog;
                bool clearOldOplog =  nodeAlreadyHoldsOplog;

                this->cluster->clusterNodes->sendRequest(nodeIdToSendRequest, createMovePartitionOplogRequest(
                        oldOplog, newOplogId, actualOplog, applyNewOplog, clearOldOplog));
            }
        }
    }

    void sendSelfOplogToPrevNode(memdbNodeId_t prevNodeId) {
        std::vector<OperationBody> selfOplog = this->operationLog->getFromDisk(OperationLogOptions{.operationLogId = 0});
        this->cluster->clusterNodes->sendRequest(prevNodeId, createMovePartitionOplogRequest(
                0, 0, selfOplog, true, false));
    }

    Request createMovePartitionOplogRequest(int oldOplog, int newOplogId, const std::vector<OperationBody>& oplog, bool applyNewOplog, bool clearOldOplog) {
        auto serialized = operationLogSerializer.serializeAllShared(oplog);

        OperationBody operationBody{};
        operationBody.operatorNumber = 0x06; //MovePartitionOplogOperator
        operationBody.flag1 = applyNewOplog; //applyNewOplog
        operationBody.flag2 = clearOldOplog; //clearOldOplog

        args_t args = OperationBody::createOperationBodyArg();
        args->push_back(SimpleString<memDbDataLength_t>::fromNumber(newOplogId));
        args->push_back(SimpleString<memDbDataLength_t>::fromNumber(oldOplog));
        args->push_back(SimpleString<memDbDataLength_t>::fromVector(*serialized));

        Request request{};
        request.operation = operationBody;

        return request;
    }

    void updateNeighbors() {
        memdbNodeId_t selfNodeId = this->cluster->selfNode->nodeId;
        std::vector<node_t> otherNodes = cluster->clusterManager->getAllNodes(selfNodeId)
                .getAllNodesExcept(selfNodeId);

        this->partitionNeighborsNodesGroupSetter.setFromOtherNodes(this->cluster, otherNodes);
    }
};