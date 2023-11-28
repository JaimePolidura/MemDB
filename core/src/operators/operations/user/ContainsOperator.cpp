#include "ContainsOperator.h"

Response ContainsOperator::operate(const OperationBody&operation, const OperationOptions options, OperatorDependencies&dependencies) {
    SimpleString<memDbDataLength_t> key = operation.getArg(0);
    memdbPartitionId_t partitionId = dependencies.cluster->getPartitionIdByKey(key);
    memDbDataStoreMap_t store = dependencies.memDbStores->getByPartitionId(partitionId);

    bool contained = store->contains(key);

    return ResponseBuilder::builder()
        .isSuccessful(contained, ErrorCode::UNKNOWN_KEY)
        ->build();
}

OperatorDescriptor ContainsOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_READ,
        .number = OperatorNumbers::CONTAINS,
        .name = "CONTAINS",
        .authorizedToExecute = { AuthenticationType::USER },
    };
}
