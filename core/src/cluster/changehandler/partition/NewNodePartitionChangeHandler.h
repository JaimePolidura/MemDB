#pragma once

#include "cluster/partitions/PartitionNeighborsNodesGroupSetter.h"
#include "cluster/Cluster.h"

#include "persistence/serializers/OperationLogSerializer.h"
#include "persistence/utils/OperationLogUtils.h"
#include "persistence/utils/OperationLogInvalidator.h"

#include "messages/request/RequestBuilder.h"
#include "operators/OperatorDispatcher.h"
#include "logging/Logger.h"

class NewNodePartitionChangeHandler {
private:
    PartitionNeighborsNodesGroupSetter partitionNeighborsNodesGroupSetter;
    OperationLogInvalidator operationLogInvalidator;
    OperationLogSerializer operationLogSerializer;

    operatorDispatcher_t operatorDispatcher;
    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

    struct CreateMoveOplogReqParams {
        std::vector<MapEntry<memDbDataLength_t >>& oplog;
        bool applyNewOplog;
        int newOplogId;
    };

public:
    using splitedSelfOplog_t = std::pair<mapEntries_t<memDbDataLength_t>, mapEntries_t<memDbDataLength_t>>;

    NewNodePartitionChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher);

    NewNodePartitionChangeHandler() = default;

    void handle(node_t newNode);

private:
    void sendSelfOplogToNodes(node_t newNode);

    void recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded, const std::vector<RingEntry>& oldClockwiseNeighbors);

    splitedSelfOplog_t splitSelfOplog(std::vector<MapEntry<memDbDataLength_t>> keysSelfOplog, RingEntry newRingEntry);

    void invalidateSelfOplogNextNode(const std::vector<OperationBody>& oplogNextNode);

    void updateNeighbors();

    Request createMoveOplogRequest(CreateMoveOplogReqParams request);

    void removeKeysFromSelfNode(const std::vector<MapEntry<memDbDataLength_t>>& keysSelfOplog);
    void updateOplogIdOfNeighNodesPlusOne(std::vector<MapEntry<memDbDataLength_t>>& newOplog, const std::vector<RingEntry>& neighbors);
    void sendNewOplogToNewNode(memdbNodeId_t nodeId, std::vector<MapEntry<memDbDataLength_t>>& oplog);
};

using splitedSelfOplog_t = std::pair<mapEntries_t<memDbDataLength_t>, mapEntries_t<memDbDataLength_t>>;