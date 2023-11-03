#include "CasOperator.h"

Response CasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    auto [key, expectedValue, newValue] = this->getArgs(operation);
    uint32_t keyHash = memDbStore->calculateHash(key);
    auto onGoingPaxosRounds = dependencies.onGoingPaxosRounds;

    if(onGoingPaxosRounds->isOnGoingProposer(keyHash)){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    std::result<std::tuple<LamportClock, LamportClock>> resultPrepare = this->sendRetriesPrepares(dependencies, options.partitionId, key, newValue);
    if(resultPrepare.has_error()){
        return Response::error(ErrorCode::CAS_FAILED);
    }
    auto [prevTimestamp, nextTimestamp] = resultPrepare.get();

    multipleResponses_t acceptMultiResponse = this->sendAccept(dependencies, keyHash, options.partitionId, key, newValue, prevTimestamp, nextTimestamp);
    if(!checkIfQuorumAndAllResponsesSuccess(acceptMultiResponse, keyHash, dependencies)){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    //Success
    if(memDbStore->put(key, newValue, false, nextTimestamp.counter, nextTimestamp.nodeId)){
        onGoingPaxosRounds->updateStateProposer(keyHash, ProposerPaxosState::COMITTED);
        dependencies.operationLog->add(options.partitionId, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::SET)
            ->args({key, newValue})
            ->timestamp(nextTimestamp.counter)
            ->selfNode(dependencies.cluster->getNodeId())
            ->buildOperationBody());
        return Response::success();

    } else {
        onGoingPaxosRounds->updateStateProposer(keyHash, ProposerPaxosState::FAILED);
        return Response::error(ErrorCode::CAS_FAILED);
    }
}

std::result<std::tuple<LamportClock, LamportClock>> CasOperator::sendRetriesPrepares(OperatorDependencies& dependencies,
                                                                        int partitionId,
                                                                        SimpleString<memDbDataLength_t> key,
                                                                        SimpleString<memDbDataLength_t> expectedValue) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(partitionId);
    uint32_t keyHash = memDbStore->calculateHash(key);
    ProposerPaxosRound proposerPaxosRound = dependencies.onGoingPaxosRounds->getProposerByKeyHashOrCreate(keyHash, WAITING_FOR_PROMISE);
    LamportClock prevTimestamp = {};
    LamportClock nextTimestamp = {};

    while(true){
        std::optional<MapEntry<memDbDataLength_t>> storedInDb = memDbStore->get(key);
        if(!storedInDb.has_value() || storedInDb->value != expectedValue){
            return std::error<std::tuple<LamportClock, LamportClock>>();
        }

        prevTimestamp = storedInDb->timestamp;
        nextTimestamp = this->getNextTimestampForKey(storedInDb.value(), dependencies);

        multipleResponses_t prepareMultiResponses = this->sendPrepare(dependencies, keyHash, partitionId, key, prevTimestamp, nextTimestamp);
        if(checkIfQuorumAndAllResponsesSuccess(prepareMultiResponses, keyHash, dependencies)) {
            break;
        }

        proposerPaxosRound.retryPrepareTimer.sleep();
    }

    proposerPaxosRound.retryPrepareTimer.reset();

    return std::ok<std::tuple<LamportClock, LamportClock>>({prevTimestamp, nextTimestamp});
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
    return LamportClock{dependencies.cluster->getNodeId(), dependencies.clock->tick(mapEntry.timestamp.counter)};
}

OperatorDescriptor CasOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_CONDITIONAL_WRITE,
        .number = OperatorNumbers::CAS,
        .name = "CAS",
        .authorizedToExecute = { AuthenticationType::API },
    };
}