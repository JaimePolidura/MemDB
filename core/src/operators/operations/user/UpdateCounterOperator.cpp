#include "UpdateCounterOperator.h"

Response UpdateCounterOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memdDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    uint32_t nNodesInCluster = dependencies.cluster->getNTotalNodesInPartition();
    SimpleString<memDbDataLength_t> key = operation.args->at(0);
    bool isIncrement = operation.flag1;

    std::result<int64_t> updateCounterResult = memdDbStore->updateCounter(key, dependencies.cluster->getNodeId(),
        nNodesInCluster, isIncrement);

    if(updateCounterResult.has_error()) {
        return Response::error(ErrorCode::INVALID_TYPE);
    }

    std::optional<MapEntry<memDbDataLength_t>> counterMapEntry = memdDbStore->get(key);

    if(!options.dontBroadcastToCluster) {
        replicate(counterMapEntry->toCounter()->counter, updateCounterResult.get(), dependencies, operation);
    }

    return Response::success();
}

void UpdateCounterOperator::replicate(Counter& counter, int64_t newUpdatedValue, OperatorDependencies& dependencies, const OperationBody& operation) {
    dependencies.cluster->clusterNodes->requestPool.submit([newUpdatedValue, dependencies, counter, operation, this]() mutable -> void {
        SimpleString<memDbDataLength_t> key = operation.args->at(0);
        uint32_t partitionId = dependencies.cluster->getPartitionIdByKey(key);

        RequestBuilder replicateCounterRequest = *RequestBuilder::builder()
            .authKey(dependencies.configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->operatorNumber(OperatorNumbers::REPLICATE_COUNTER)
            ->operatorFlag1(operation.flag1); //Is increment

        multipleResponses_t responses = dependencies.cluster->clusterNodes->broadcastForEachAndWait(SendRequestOptions{
            .partitionId = static_cast<int>(partitionId),
            .canBeStoredInHint = true
        }, [replicateCounterRequest, counter, key, dependencies, newUpdatedValue](memdbNodeId_t nodeId) mutable -> OperationBody {
            const auto[lastSeenIncrement, lastSeenDecrement] = counter.getLastSeen(nodeId);

            dependencies.logger->debugInfo("Sending to node {0} REPLICATE_COUNTER(newValue = {1}, lastSeenInc = {2}, lastSeenDec = {3})",
                nodeId, newUpdatedValue, lastSeenIncrement, lastSeenDecrement);

            return replicateCounterRequest
                .addArg(key)
                ->addDoubleArg(newUpdatedValue)
                ->addDoubleArg(lastSeenIncrement)
                ->addDoubleArg(lastSeenDecrement)
                ->buildOperationBody();
        });

        responses->onResponse([key, counter, dependencies, this](const Response& response) mutable -> void {
            this->onReplicationCounterResponse(key, response, counter, dependencies);
        });
    });
}

void UpdateCounterOperator::onReplicationCounterResponse(const SimpleString<memDbDataLength_t>& key, const Response& response,
    Counter& counter, const OperatorDependencies& dependencies) {

    int64_t nIncrementToSync = response.getResponseValueAtOffset(0, 8).to<int64_t>();
    int64_t nDecrementToSync = response.getResponseValueAtOffset(8, 8).to<int64_t>();
    memdbNodeId_t otherNodeId = response.getResponseValueAtOffset(16, sizeof(memdbNodeId_t)).to<memdbNodeId_t>();

    dependencies.logger->debugInfo("Received REPLICATE_COUNTER response from node {0} with incToSync {1}, decToSync: {2}",
        otherNodeId, nIncrementToSync, nDecrementToSync);

    if(nIncrementToSync > 0) { //Needs to sync
        counter.syncIncrement(nIncrementToSync, otherNodeId);
    }
    if(nDecrementToSync > 0) {
        counter.syncDecrement(nDecrementToSync, otherNodeId);
    }
}

OperatorDescriptor UpdateCounterOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::UPDATE_COUNTER,
        .name = "UPDATE_COUNTER",
        .authorizedToExecute = { AuthenticationType::USER },
    };
}

