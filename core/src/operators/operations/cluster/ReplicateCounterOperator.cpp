#include "ReplicateCounterOperator.h"

#include "messages/response/ResponseBuilder.h"

Response ReplicateCounterOperator::operate(const OperationBody&operation, const OperationOptions options, OperatorDependencies&dependencies) {
    memDbDataStoreMap_t memdDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString<memDbDataLength_t> key = operation.args->at(0);
    int64_t newValue = operation.getDoubleArgU64(1);
    int64_t lastSeenIncrement = operation.getDoubleArgU64(3);
    int64_t lastSeenDecrement = operation.getDoubleArgU64(4);
    bool isIncrement = operation.flag1;

    std::optional<MapEntry<memDbDataLength_t>> result = memdDbStore->get(key);

    if(result.has_value() && result->type != NodeType::COUNTER) {
        return Response::error(ErrorCode::INVALID_TYPE);
    }
    if(!result.has_value()) {
        memdDbStore->putCounter(key, dependencies.cluster->getNodeId(),
            dependencies.cluster->getNTotalNodesInPartition());//Add new counter
        result = memdDbStore->get(key);
    }

    std::shared_ptr<CounterAVLNode> counterMapNode = result->toCounter();

    ReplicateCounterResponse response = counterMapNode->counter.onReplicationCounter(ReplicateCounterRequest{
        .otherNodeId = operation.nodeId,
        .isIncrement = isIncrement,
        .newValue = newValue,
        .lastSeenSelfIncrement = lastSeenIncrement,
        .lastSeenSelfDecrement = lastSeenDecrement
    });

    dependencies.logger->debugInfo("Recevied REPLICATE_COUNTER from node {0} with new value: {1}, lastSeenInc: {2}, "
                                   "lastSeenDec: {3}. Returned incToSync: {4}, decToSync: {5}", operation.nodeId, newValue,
                                   lastSeenIncrement, lastSeenDecrement, response.nIncrementToSync, response.nDecrementToSync);

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
        .authorizedToExecute = { AuthenticationType::NODE },
        .properties = {}
    };
}