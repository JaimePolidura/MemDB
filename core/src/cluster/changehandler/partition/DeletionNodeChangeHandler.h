#pragma once

#include "cluster/changehandler/partition/MoveOpLogRequestCreator.h"
#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"
#include "cluster/Cluster.h"

#include "persistence/serializers/OperationLogSerializer.h"
#include "persistence/utils/OperationLogUtils.h"
#include "persistence/utils/OperationLogInvalidator.h"

#include "operators/OperatorDispatcher.h"
#include "logging/Logger.h"

class DeletionNodeChangeHandler {
private:
    PartitionNeighborsNodesGroupSetter partitionNeighborsNodesGroupSetter;
    OperationLogSerializer operationLogSerializer;

    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

    MoveOpLogRequestCreator moveOpLogRequestCreator;

public:
    DeletionNodeChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher);

    void handle(node_t deletedNode);

private:
    void sendRestOplogsToNextNodes(const std::vector<RingEntry>& neighborsClockWise);

    void sendSelfOplogToPrevNode(memdbNodeId_t prevNodeId);

    void updateNeighbors();
};