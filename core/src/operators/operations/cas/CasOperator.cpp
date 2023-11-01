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

    multipleResponses_t prepareMultiResponses = dependencies.cluster->broadcastAndWait({.partitionId = options.partitionId}, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::CAS_PREPARE)
            ->selfNode(dependencies.cluster->getNodeId())
            ->addArg(key)
            ->addArg(expectedValue)
            ->addArg(newValue)
            ->addArg(SimpleString<memDbDataLength_t >::fromNumber(dependencies.cluster->getNodeId()))
            ->addDoubleArg(clock.counter.load())
            ->buildOperationBody());

    if(!prepareMultiResponses->waitForQuorum(dependencies.configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS))) {
        return Response::error(ErrorCode::CAS_FAILED);
    }

    std::map<memdbNodeId_t, Response> promiseResponses = prepareMultiResponses->getResponses();

    if(promiseResponsesHasGreaterTimestamp(clock, promiseResponses)) {
        return Response::error(ErrorCode::CAS_FAILED);
    }

    multipleResponses_t acceptMultiResponse = dependencies.cluster->broadcastAndWait({.partitionId = options.partitionId}, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::CAS_ACCEPT)
            ->selfNode(dependencies.cluster->getNodeId())
            ->addArg(key)
            ->addArg(expectedValue)
            ->addArg(newValue)
            ->addArg(SimpleString<memDbDataLength_t >::fromNumber(clock.nodeId))
            ->addDoubleArg(clock.counter.load())
            ->buildOperationBody());

    if(!acceptMultiResponse->waitForQuorum(dependencies.configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS))) {
        return Response::error(ErrorCode::CAS_FAILED);
    }

    //TODO

    return Response::success();
}

bool CasOperator::promiseResponsesHasGreaterTimestamp(const LamportClock& clock, const std::map<memdbNodeId_t, Response>& prepareResponses) {
    for (const auto[nodeId, response] : prepareResponses) {
        if(!response.isSuccessful){
           return true;
        }
    }

    return false;
}

OperatorDescriptor CasOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_CONDITIONAL_WRITE,
        .number = OperatorNumbers::CAS,
        .name = "CAS",
        .authorizedToExecute = { AuthenticationType::API },
    };
}