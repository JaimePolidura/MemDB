#include "operators/operations/user/GetOperator.h"

Response GetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memdDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    std::optional<MapEntry<memDbDataLength_t>> result = memdDbStore->get(operation.args->at(0));

    return ResponseBuilder::builder()
        .isSuccessful(result.has_value(), ErrorCode::UNKNOWN_KEY)
        ->timestamp(this->getTimestampCounter(result))
        ->value(this->getValue(result))
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

LamportClock GetOperator::getTimestampCounter(const std::optional<MapEntry<memDbDataLength_t>>& entry) {
    if(!entry.has_value() || entry->type == NodeType::COUNTER) {
        return LamportClock{0, 0};
    } else {
        return entry->toData()->timestamp;
    }
}

SimpleString<memDbDataLength_t> GetOperator::getValue(const std::optional<MapEntry<memDbDataLength_t>>& entry) {
    if(!entry.has_value()) {
        return SimpleString<memDbDataLength_t>::fromString("");
    }

    if(entry->type == NodeType::COUNTER) {
        return SimpleString<memDbDataLength_t>::fromNumber(entry->toCounter().get());
    } else {
        return entry->toData()->value;
    }
}