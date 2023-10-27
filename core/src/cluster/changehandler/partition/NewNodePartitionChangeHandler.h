#pragma once

#include "cluster/changehandler/partition/MoveOpLogRequestCreator.h"
#include "cluster/partitions/PartitionNeighborsNodesSetter.h"
#include "cluster/Cluster.h"

#include "persistence/serializers/OperationLogSerializer.h"
#include "persistence/utils/OperationLogUtils.h"
#include "persistence/utils/OperationLogInvalidator.h"

#include "messages/request/RequestBuilder.h"
#include "operators/OperatorDispatcher.h"
#include "logging/Logger.h"

class NewNodePartitionChangeHandler {
private:
    PartitionNeighborsNodesSetter partitionNeighborsNodesGroupSetter;
    OperationLogInvalidator operationLogInvalidator;
    OperationLogSerializer operationLogSerializer;
    operatorDispatcher_t operatorDispatcher;

    operationLog_t operationLog;
    cluster_t cluster;
    logger_t logger;

    MoveOpLogRequestCreator moveOpLogRequestCreator;

public:
    using splitedSelfOplog_t = std::pair<mapEntries_t<memDbDataLength_t>, mapEntries_t<memDbDataLength_t>>;

    NewNodePartitionChangeHandler(logger_t logger, cluster_t cluster, operationLog_t operationLog, operatorDispatcher_t operatorDispatcher);

    void handle(node_t newNode);

private:
    void sendSelfOplogToNodes(node_t newNode);

    void recomputeSelfOplogAndSendNextNode(RingEntry newRingEntryAdded, const std::vector<RingEntry>& oldClockwiseNeighbors);

    splitedSelfOplog_t splitSelfOplog(std::vector<MapEntry<memDbDataLength_t>> keysSelfOplog, RingEntry newRingEntry);

    void invalidateSelfOplogNextNode(const std::vector<OperationBody>& oplogNextNode);

    void removeKeysFromSelfNode(const std::vector<MapEntry<memDbDataLength_t>>& keysSelfOplog);
    void updateOplogIdOfNeighNodesPlusOne(std::vector<MapEntry<memDbDataLength_t>>& newOplog, const std::vector<RingEntry>& neighbors);
    void sendNewOplogToNewNode(memdbNodeId_t nodeId, std::vector<MapEntry<memDbDataLength_t>>& oplog);
};

using splitedSelfOplog_t = std::pair<mapEntries_t<memDbDataLength_t>, mapEntries_t<memDbDataLength_t>>;