#pragma once

#include "logging/Logger.h"
#include "cluster/Cluster.h"
#include "operators/OperatorDispatcher.h"
#include "persistence/OperationLogSerializer.h"
#include "persistence/OperationLogUtils.h"
#include "persistence/OperationLogInvalidator.h"
#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"

class DeletionNodeChangeHandler {
private:
    PartitionNeighborsNodesGroupSetter partitionNeighborsNodesGroupSetter;
    OperationLogSerializer operationLogSerializer;

    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

public:
    DeletionNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher);

    DeletionNodeChangeHandler() = default;

    void handle(node_t deletedNode);

private:
    void sendRestOplogsToNextNodes(const std::vector<RingEntry>& neighborsClockWise);

    void sendSelfOplogToPrevNode(memdbNodeId_t prevNodeId);

    Request createMovePartitionOplogRequest(int oldOplog, int newOplogId, const std::vector<OperationBody>& oplog, bool applyNewOplog, bool clearOldOplog);

    void updateNeighbors();
};