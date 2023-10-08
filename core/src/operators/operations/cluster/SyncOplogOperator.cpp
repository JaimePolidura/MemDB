#include "operators/operations/cluster/SyncOplogOperator.h"

Response SyncOplogOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    uint64_t lastTimestampUnsync = parseUnsyncTimestampFromRequest(operation);
    uint32_t nodeOplogIdToSync = calculateSelfOplogIdFromNodeOplogId(operation, dependencies) ;

    std::vector<OperationBody> unsyncedOplog = dependencies.operationLog->getAfterTimestamp(lastTimestampUnsync, OperationLogOptions{
            .operationLogId = nodeOplogIdToSync
    });

    unsyncedOplog = this->operationLogCompacter.compact(unsyncedOplog);

    std::vector<uint8_t> serializedUnsyncedOpLog = this->operationLogSerializer.serializeAll(unsyncedOplog);

    return Response::success(SimpleString<memDbDataLength_t>::fromVector(serializedUnsyncedOpLog));
}

constexpr OperatorDescriptor SyncOplogOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OPERATOR_NUMBER,
            .name = "SYNC",
            .authorizedToExecute = { AuthenticationType::NODE },
            .isMulti = true,
    };
}

//Timestamp is 64 bits Actual memdb data size is 32 bits. Doest fit, we pass two args that consist of the two parts
uint64_t SyncOplogOperator::parseUnsyncTimestampFromRequest(const OperationBody &operation) const {
    auto part1 = operation.args->at(0).to<uint32_t>();
    auto part2 = operation.args->at(1).to<uint32_t>();

    return ((uint64_t) part1) << 32 | part2;
}

uint32_t SyncOplogOperator::calculateSelfOplogIdFromNodeOplogId(const OperationBody &body, OperatorDependencies dependencies) {
    auto nodeOplogId = body.getArg(2).to<uint32_t>();

    if(!dependencies.configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
        return nodeOplogId;
    }

    memdbNodeId_t otherNodeId = body.nodeId;
    int distance = dependencies.cluster->getPartitionObject()->getDistance(otherNodeId);

    return nodeOplogId - distance;
}