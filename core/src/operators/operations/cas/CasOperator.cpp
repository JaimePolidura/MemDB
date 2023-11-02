#include "CasOperator.h"

Response CasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);

    auto [key, expectedValue, newValue] = this->getArgs(operation);
    std::optional<MapEntry<memDbDataLength_t>> storedInDb = memDbStore->get(key);

    if(!storedInDb.has_value() || storedInDb->value != expectedValue){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    LamportClock nextTimestamp = this->getNextTimestampForKey(storedInDb.value(), dependencies);
    LamportClock prevTimestamp = storedInDb->timestamp;

    multipleResponses_t prepareMultiResponses = this->sendPrepare(dependencies.cluster, options.partitionId, key, prevTimestamp, nextTimestamp);

    if(!prepareMultiResponses->waitForQuorum(dependencies.configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS))) {
        return Response::error(ErrorCode::CAS_FAILED);
    }
    if(!prepareMultiResponses->allResponsesSuccessful()){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    multipleResponses_t acceptMultiResponse = this->sendAccept(dependencies.cluster, options.partitionId, key, newValue, prevTimestamp, nextTimestamp);

    if(!acceptMultiResponse->waitForQuorum(dependencies.configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS))) {
        return Response::error(ErrorCode::CAS_FAILED);
    }
    if(!acceptMultiResponse->allResponsesSuccessful()){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    bool success = memDbStore->put(key, newValue, false, nextTimestamp.counter, nextTimestamp.nodeId);

    if(success){
        dependencies.onGoingPaxosRounds->updateAcceptedTimestamp(memDbStore->calculateHash(key), nextTimestamp, newValue);
    }

    return ResponseBuilder::builder()
            .isSuccessful(success, ErrorCode::CAS_FAILED)
            ->build();
}

multipleResponses_t CasOperator::sendPrepare(cluster_t cluster,
                                int partitionId,
                                SimpleString<memDbDataLength_t> key,
                                LamportClock prevTimestamp,
                                LamportClock nextTimestamp) {
    return cluster->broadcastAndWait({.partitionId = partitionId}, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::CAS_PREPARE)
            ->selfNode(cluster->getNodeId())
            ->addArg(key)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(prevTimestamp.nodeId))
            ->addDoubleArg(prevTimestamp.counter)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(cluster->getNodeId()))
            ->addDoubleArg(nextTimestamp.counter.load())
            ->buildOperationBody());
}

multipleResponses_t CasOperator::sendAccept(cluster_t cluster,
                               int partitionId,
                               SimpleString<memDbDataLength_t> key,
                               SimpleString<memDbDataLength_t> value,
                               LamportClock prevTimestamp,
                               LamportClock nextTimestamp) {
    return cluster->broadcastAndWait({.partitionId = partitionId}, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::CAS_ACCEPT)
            ->selfNode(cluster->getNodeId())
            ->addArg(key)
            ->addArg(value)
            ->addArg(SimpleString<memDbDataLength_t >::fromNumber(prevTimestamp.nodeId))
            ->addDoubleArg(prevTimestamp.counter.load())
            ->addArg(SimpleString<memDbDataLength_t >::fromNumber(cluster->getNodeId()))
            ->addDoubleArg(nextTimestamp.counter.load())
            ->buildOperationBody());
}

std::tuple<SimpleString<memDbDataLength_t>, SimpleString<memDbDataLength_t>, SimpleString<memDbDataLength_t>> getArgs(const OperationBody& operation) {
    return std::make_tuple(operation.getArg(0), operation.getArg(1), operation.getArg(3));
}

LamportClock CasOperator::getNextTimestampForKey(MapEntry<memDbDataLength_t> mapEntry,
                                                OperatorDependencies& dependencies) {
    return LamportClock{dependencies.cluster->getNodeId(), mapEntry.timestamp.counter + 1};
}

OperatorDescriptor CasOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_CONDITIONAL_WRITE,
        .number = OperatorNumbers::CAS,
        .name = "CAS",
        .authorizedToExecute = { AuthenticationType::API },
    };
}