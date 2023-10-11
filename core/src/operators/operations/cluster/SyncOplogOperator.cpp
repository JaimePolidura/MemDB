#include "operators/operations/cluster/SyncOplogOperator.h"

iterator_t SyncOplogOperator::createMultiResponseSenderIterator(const OperationBody& operation, OperatorDependencies& dependencies) {
    uint64_t lastTimestampUnsync = operation.getDoubleArgU64(1);
    uint32_t nodeOplogIdToSync = calculateSelfOplogIdFromNodeOplogId(operation, dependencies);

    return std::dynamic_pointer_cast<Iterator>(dependencies.operationLog->getAfterTimestamp(lastTimestampUnsync, OperationLogOptions{
            .operationLogId = nodeOplogIdToSync
    }));
}

Response SyncOplogOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    return Response::error(ErrorCode::INVALID_CALL);
}

constexpr OperatorDescriptor SyncOplogOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::SYNC_OPLOG,
            .name = "SYNC",
            .authorizedToExecute = { AuthenticationType::NODE },
    };
}

uint32_t SyncOplogOperator::calculateSelfOplogIdFromNodeOplogId(const OperationBody &body, OperatorDependencies dependencies) {
    auto nodeOplogId = body.getArg(3).to<uint32_t>();

    if(!dependencies.configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
        return nodeOplogId;
    }

    memdbNodeId_t otherNodeId = body.nodeId;
    int distance = dependencies.cluster->getPartitionObject()->getDistance(otherNodeId);

    return nodeOplogId - distance;
}