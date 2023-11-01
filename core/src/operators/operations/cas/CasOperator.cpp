#include "CasOperator.h"

Response CasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString<memDbDataLength_t> key = operation.getArg(0);
    SimpleString<memDbDataLength_t> expectedValue = operation.getArg(1);
    SimpleString<memDbDataLength_t> newValue = operation.getArg(2);

    std::optional<MapEntry<memDbDataLength_t>> storedInDb = memDbStore->get(key);

    if(!storedInDb.has_value() || storedInDb->value != expectedValue){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    LamportClock clock = storedInDb->timestamp;

    dependencies.cluster->broadcast2();
}

OperatorDescriptor CasOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_CONDITIONAL_WRITE,
        .number = OperatorNumbers::CAS,
        .name = "CAS",
        .authorizedToExecute = { AuthenticationType::API },
    };
};