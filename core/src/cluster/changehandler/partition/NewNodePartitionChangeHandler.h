#pragma once

#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"
#include "cluster/Cluster.h"

#include "logging/Logger.h"

#include "operators/OperatorDispatcher.h"

#include "persistence/utils/OperationLogSerializer.h"
#include "persistence/utils/OperationLogUtils.h"
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
    NewNodePartitionChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher);

    NewNodePartitionChangeHandler() = default;

    void handle(node_t newNode);

private:
    void sendSelfOplogToNodes(node_t newNode);

    void recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded, const std::vector<RingEntry>& oldClockwiseNeighbors);

    std::pair<std::vector<OperationBody>, std::vector<OperationBody>> splitSelfOplog(RingEntry newRingEntry);

    void invalidateSelfOplogNextNode(const std::vector<OperationBody>& oplogNextNode);

    void updateNeighbors();

    Request createMovePartitionOplogRequest(int newOplogId, const std::vector<OperationBody>& oplog, bool applyNewOplog);
};