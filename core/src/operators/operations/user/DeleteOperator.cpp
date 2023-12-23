#include "operators/operations/user/DeleteOperator.h"

Response DeleteOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t dbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString<memDbDataLength_t> key = operation.getArg(0);
    LamportClock requestTimestamp = this->getTimestamp(operation, options, dependencies);
    std::result<DbEditResult> resultRemove = dbStore->remove(key, requestTimestamp, options.updateClockStrategy,
                                                             dependencies.clock, options.checkTimestamps);

    dependencies.logger->debugInfo("Executed DEL({0}) Success? {1} Req timestamp: {2} New timestamp: {3}", key.toString(),
                                   resultRemove.is_success(), requestTimestamp.toString(), resultRemove->timestampOfOperation);

    return ResponseBuilder::builder()
        .isSuccessful(resultRemove.is_success(), ErrorCode::UNKNOWN_KEY)
        ->timestampCounter(resultRemove.is_success() ? resultRemove->timestampOfOperation : 0)
        ->build();
}

OperatorDescriptor DeleteOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_WRITE,
        .number = OperatorNumbers::DEL,
        .name = "DELETE",
        .authorizedToExecute = { AuthenticationType::USER, AuthenticationType::NODE },
        .properties = { OperatorProperty::BROADCAST, OperatorProperty::PERSISTENCE }
    };
}

LamportClock DeleteOperator::getTimestamp(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    if(options.fromClient()) {
        return LamportClock{dependencies.cluster->getNodeId(), operation.timestamp};
    } else {
        return LamportClock{operation.nodeId, operation.timestamp};
    }
}