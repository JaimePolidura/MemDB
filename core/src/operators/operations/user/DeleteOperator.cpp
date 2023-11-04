#include "operators/operations/user/DeleteOperator.h"

Response DeleteOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t dbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    std::result<DbEditResult> resultRemove = dbStore->remove(operation.args->at(0), LamportClock{operation.nodeId, operation.timestamp},
                                   options.updateClockStrategy, dependencies.clock);

    return ResponseBuilder::builder()
        .isSuccessful(resultRemove.is_success(), ErrorCode::UNKNOWN_KEY)
        ->timestamp(resultRemove->timestampOfOperation)
        ->build();
}

OperatorDescriptor DeleteOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_WRITE,
        .number = OperatorNumbers::DEL,
        .name = "DELETE",
        .authorizedToExecute = { AuthenticationType::API, AuthenticationType::NODE },
    };
}
