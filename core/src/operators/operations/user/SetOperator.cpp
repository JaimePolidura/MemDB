#include "operators/operations/user/SetOperator.h"

Response SetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString key = operation.args->at(0);
    SimpleString value = operation.args->at(1);
    LamportClock requestTimestamp = LamportClock{operation.nodeId, operation.timestamp};

    std::result<DbEditResult> resultSet = memDbStore->put(key, value, requestTimestamp, options.updateClockStrategy,
                                                          dependencies.clock, options.checkTimestamps);

    dependencies.logger->debugInfo("Executed SET({0}, {1}) Success? {2} Req timestamp: {3} New timestamp: {4}",key.toString(), value.toString(),
                                   resultSet.is_success(), requestTimestamp.toString(), resultSet->timestampOfOperation);

    return ResponseBuilder::builder()
        .isSuccessful(resultSet.is_success(), ErrorCode::ALREADY_REPLICATED)
        ->timestamp(resultSet->timestampOfOperation)
        ->build();
}

OperatorDescriptor SetOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::DB_STORE_WRITE,
            .number = OperatorNumbers::SET,
            .name = "SET",
            .authorizedToExecute = { AuthenticationType::API, AuthenticationType::NODE },
    };
}