#include "operators/operations/user/SetOperator.h"

Response SetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString key = operation.args->at(0);
    SimpleString value = operation.args->at(1);

    std::result<DbEditResult> resultSet = memDbStore->put(key, value, LamportClock{operation.nodeId, operation.timestamp},
                                   options.updateClockStrategy, dependencies.clock);

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