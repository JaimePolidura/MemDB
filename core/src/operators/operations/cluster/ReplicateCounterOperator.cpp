#include "ReplicateCounterOperator.h"

#include "messages/response/ResponseBuilder.h"

Response ReplicateCounterOperator::operate(const OperationBody&operation, const OperationOptions options, OperatorDependencies&dependencies) {
    memDbDataStoreMap_t memdDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString<memDbDataLength_t> key = operation.args->at(0);
    uint64_t newValue = operation.getDoubleArgU64(1);
    uint64_t lastSeenIncrement = operation.getDoubleArgU64(3);
    uint64_t lastSeenDecrement = operation.getDoubleArgU64(4);

    std::optional<MapEntry<memDbDataLength_t>> result = memdDbStore->get(key);

    if(result.has_value() && result->type != NodeType::COUNTER) {
        return Response::error(ErrorCode::INVALID_TYPE);
    }

    std::shared_ptr<CounterAVLNode> counterMapNode = result->toCounter();

    ReplicateCounterResponse response = counterMapNode->counter.onReplicationCounter(ReplicateCounterRequest{
        .otherNodeId = operation.nodeId,
        .isIncrement = operation.flag1,
        .newValue = newValue,
        .lastSeenSelfIncrement = lastSeenIncrement,
        .lastSeenSelfDecrement = lastSeenDecrement
    });

    return ResponseBuilder::builder()
        .success()
        ->values( {
            SimpleString<memDbDataLength_t>::fromVector(Utils::toBuffer(
            response.needsIncrementSync ? response.nIncrementToSync : 0,
                response.needsDecrementSync ? response.nDecrementToSync : 0)),
            SimpleString<memDbDataLength_t>::fromNumber(dependencies.cluster->getNodeId())
        })
        ->build();

    return Response::success();
}

OperatorDescriptor ReplicateCounterOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_WRITE,
        .number = OperatorNumbers::REPLICATE_COUNTER,
        .name = "REPLICATE_COUNTER",
        .authorizedToExecute = { AuthenticationType::NODE }
    };
}

