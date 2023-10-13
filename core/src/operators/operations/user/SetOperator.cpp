#include "operators/operations/user/SetOperator.h"

Response SetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString key = operation.args->at(0);
    SimpleString value = operation.args->at(1);

    bool ignoreTimestamps = !options.checkTimestamps;
    bool updated = memDbStore->put(key, value, ignoreTimestamps, operation.timestamp, operation.nodeId);

    return updated ?
        Response::success() :
        Response::error(ErrorCode::ALREADY_REPLICATED);
}

OperatorDescriptor SetOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::DB_STORE_WRITE,
            .number = OperatorNumbers::SET,
            .name = "SET",
            .authorizedToExecute = { AuthenticationType::API, AuthenticationType::NODE },
    };
}