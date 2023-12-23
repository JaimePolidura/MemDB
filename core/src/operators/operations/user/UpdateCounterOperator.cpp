#include "UpdateCounterOperator.h"

Response UpdateCounterOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memdDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    SimpleString<memDbDataLength_t> key = operation.args->at(0);
    bool isIncrement = operation.flag1;
    uint32_t nNodesInCluster = this->getNNodesInCluster(dependencies);

    std::result<uint64_t> updateCounterResult = memdDbStore->updateCounter(key, dependencies.cluster->getNodeId(),
        nNodesInCluster, isIncrement);

    if(updateCounterResult.has_error()) {
        return Response::error(ErrorCode::INVALID_TYPE);
    }

    std::optional<MapEntry<memDbDataLength_t>> counterMapEntry = memdDbStore->get(key);

    replicate(counterMapEntry->toCounter()->counter, updateCounterResult.get(), dependencies, operation);

    return Response::success();
}

void UpdateCounterOperator::replicate(Counter& counter, uint64_t newUpdatedValue, OperatorDependencies& dependencies, const OperationBody& operation) {
    dependencies.cluster->clusterNodes->requestPool.submit([newUpdatedValue, dependencies, counter, operation, this]() {
        SimpleString<memDbDataLength_t> key = operation.args->at(0);
        uint32_t partitionId = dependencies.cluster->getPartitionIdByKey(key);

        RequestBuilder * replicateCounterRequest = RequestBuilder::builder()
            .authKey(dependencies.configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->operatorNumber(OperatorNumbers::REPLICATE_COUNTER)
            ->operatorFlag1(operation.flag1); //Is increment

        multipleResponses_t responses = dependencies.cluster->clusterNodes->broadcastForEachAndWait(SendRequestOptions{
            .partitionId = static_cast<int>(partitionId),
            .canBeStoredInHint = true
        }, [replicateCounterRequest, counter, key, newUpdatedValue](memdbNodeId_t nodeId) -> OperationBody {
            const auto[lastSeenIncrement, lastSeenDecrement] = counter.getLastSeen(nodeId);

            return replicateCounterRequest
                ->addArg(key)
                ->addDoubleArg(newUpdatedValue)
                ->addDoubleArg(lastSeenIncrement)
                ->addDoubleArg(lastSeenDecrement)
                ->buildOperationBody();
        });

        responses->onResponse([key, counter, this](const Response& response) mutable -> void {
            this->onReplicationCounterResponse(key, response, counter);
        });
    });
}

void UpdateCounterOperator::onReplicationCounterResponse(const SimpleString<memDbDataLength_t>& key, const Response& response, Counter& counter) {
    memdbNodeId_t otherNodeId = response.getResponseValueAtOffset(8, sizeof(memdbNodeId_t)).to<memdbNodeId_t>();
    uint64_t nIncrementToSync = response.getResponseValueAtOffset(0, 8).to<uint64_t>();
    uint64_t nDecrementToSync = response.getResponseValueAtOffset(8, 8).to<uint64_t>();

    if(nIncrementToSync > 0) { //Needs to sync
        counter.syncIncrement(nIncrementToSync, otherNodeId);
    }
    if(nDecrementToSync > 0) {
        counter.syncDecrement(nDecrementToSync, otherNodeId);
    }
}

uint32_t UpdateCounterOperator::getNNodesInCluster(OperatorDependencies& dependencies) {
    bool usingPartitions = dependencies.configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS);
    bool usingReplication = dependencies.configuration->getBoolean(ConfigurationKeys::USE_REPLICATION);

    if(!usingReplication) {
        return 1;
    }
    if(!usingPartitions) {
        return dependencies.cluster->clusterNodes->getSize();
    }
    if(usingPartitions) {
        return dependencies.cluster->getNodesPerPartition();
    }

    return 1;
}

OperatorDescriptor UpdateCounterOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_WRITE,
        .number = OperatorNumbers::UPDATE_COUNTER,
        .name = "UPDATE_COUNTER",
        .authorizedToExecute = { AuthenticationType::NODE },
    };
}

