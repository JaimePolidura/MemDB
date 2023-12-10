#include "operators/operations/user/GetOperator.h"

Response GetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memdDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    std::optional<MapEntry<memDbDataLength_t>> result = memdDbStore->get(operation.args->at(0));

    return ResponseBuilder::builder()
        .isSuccessful(result.has_value(), ErrorCode::UNKNOWN_KEY)
        ->timestamp(result->timestamp)
        ->value(result->value)
        ->build();
}

OperatorDescriptor GetOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::DB_STORE_READ,
            .number = OperatorNumbers::GET,
            .name = "GET",
            .authorizedToExecute = { AuthenticationType::USER },
    };
}