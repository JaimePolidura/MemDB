#include "CasOperator.h"

Response CasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    auto [key, expectedValue, newValue] = this->getArgs(operation);
    uint32_t keyHash = memDbStore->calculateHash(key);
    auto onGoingPaxosRounds = dependencies.onGoingPaxosRounds;
    ProposerPaxosRound proposerPaxosRound = onGoingPaxosRounds->getProposerByKeyHashOrCreate(keyHash, ProposerPaxosState::WAITING_FOR_PROMISE);

    if(onGoingPaxosRounds->isOnGoingProposer(keyHash)){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    LamportClock prevTimestamp = {};
    LamportClock nextTimestamp = {};
    
    //Extract function
    while(true){
        std::optional<MapEntry<memDbDataLength_t>> storedInDb = memDbStore->get(key);
        if(!storedInDb.has_value() || storedInDb->value != expectedValue){
            return Response::error(ErrorCode::CAS_FAILED);
        }

        prevTimestamp = storedInDb->timestamp;
        nextTimestamp = this->getNextTimestampForKey(storedInDb.value(), dependencies);

        multipleResponses_t prepareMultiResponses = this->sendPrepare(dependencies, keyHash, options.partitionId, key, prevTimestamp, nextTimestamp);
        if(checkIfQuorumAndAllResponsesSuccess(prepareMultiResponses, keyHash, dependencies)) {
            break;
        }

        proposerPaxosRound.retryPrepareTimer.sleep();
    }

    proposerPaxosRound.retryPrepareTimer.reset();

    multipleResponses_t acceptMultiResponse = this->sendAccept(dependencies, keyHash, options.partitionId, key, newValue, prevTimestamp, nextTimestamp);
    if(!checkIfQuorumAndAllResponsesSuccess(acceptMultiResponse, keyHash, dependencies)){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    bool success = memDbStore->put(key, newValue, false, nextTimestamp.counter, nextTimestamp.nodeId);

    if(success){
        onGoingPaxosRounds->updateStateProposer(keyHash, ProposerPaxosState::COMITTED);
        dependencies.operationLog->add(options.partitionId, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::SET)
            ->args({key, newValue})
            ->timestamp(nextTimestamp.counter)
            ->selfNode(dependencies.cluster->getNodeId())
            ->buildOperationBody());
    } else {
        onGoingPaxosRounds->updateStateProposer(keyHash, ProposerPaxosState::FAILED);
    }

    return ResponseBuilder::builder()
            .isSuccessful(success, ErrorCode::CAS_FAILED)
            ->build();
}

bool CasOperator::checkIfQuorumAndAllResponsesSuccess(multipleResponses_t multiResponses, uint32_t keyHash, OperatorDependencies& dependencies) {
    if(!multiResponses->waitForSuccessfulQuorum(dependencies.configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS))) {
        dependencies.onGoingPaxosRounds->updateStateProposer(keyHash, ProposerPaxosState::FAILED);
        return false;
    }
    return true;
}

multipleResponses_t CasOperator::sendPrepare(OperatorDependencies& dependencies,
                                             uint32_t keyHash,
                                             int partitionId,
                                             SimpleString<memDbDataLength_t> key,
                                             LamportClock prevTimestamp,
                                             LamportClock nextTimestamp) {
    dependencies.onGoingPaxosRounds->updateStateProposer(keyHash, ProposerPaxosState::WAITING_FOR_PROMISE);

    return dependencies.cluster->broadcastAndWait({.partitionId = partitionId}, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::CAS_PREPARE)
            ->selfNode(dependencies.cluster->getNodeId())
            ->addArg(key)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(prevTimestamp.nodeId))
            ->addDoubleArg(prevTimestamp.counter)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(dependencies.cluster->getNodeId()))
            ->addDoubleArg(nextTimestamp.counter.load())
            ->buildOperationBody());
}

multipleResponses_t CasOperator::sendAccept(OperatorDependencies& dependencies,
                                            uint32_t keyHash,
                                            int partitionId,
                                            SimpleString<memDbDataLength_t> key,
                                            SimpleString<memDbDataLength_t> value,
                                            LamportClock prevTimestamp,
                                            LamportClock nextTimestamp) {
    dependencies.onGoingPaxosRounds->updateStateProposer(keyHash, ProposerPaxosState::WAITING_FOR_ACCEPT);

    return dependencies.cluster->broadcastAndWait({.partitionId = partitionId}, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::CAS_ACCEPT)
            ->selfNode(dependencies.cluster->getNodeId())
            ->addArg(key)
            ->addArg(value)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(prevTimestamp.nodeId))
            ->addDoubleArg(prevTimestamp.counter.load())
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(dependencies.cluster->getNodeId()))
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