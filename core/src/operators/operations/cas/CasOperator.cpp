#include "CasOperator.h"

Response CasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    auto [key, expectedValue, newValue] = getArgs(operation);
    uint32_t keyHash = memDbStore->calculateHash(key);
    proposerPaxosRound_t proposerPaxosRound = dependencies.onGoingPaxosRounds->getProposerByKeyHash(keyHash, ProposerPaxosState::WAITING_FOR_PROMISE);
    std::unique_lock<std::mutex> uniqueLock(proposerPaxosRound->proposerLock);

    dependencies.logger->debugInfo("Received CAS(key = {0}, expectedValue = {1}, newValue = {2})", key.toString(), expectedValue.toString(), newValue.toString());

    std::result<std::tuple<LamportClock, LamportClock>> resultPrepare = sendRetriesPrepares(dependencies, proposerPaxosRound, options.partitionId, key, expectedValue);
    if(resultPrepare.has_error()){
        return Response::error(ErrorCode::CAS_FAILED);
    }
    auto [prevTimestamp, nextTimestamp] = resultPrepare.get();

    multipleResponses_t acceptMultiResponse = sendAccept(dependencies, proposerPaxosRound, options.partitionId, key, newValue, prevTimestamp, nextTimestamp);
    if(!checkIfQuorumAndAllResponsesSuccess(acceptMultiResponse, proposerPaxosRound, dependencies)) { //TODO Move to method like sendRetriesPrepares
        proposerPaxosRound->state = ProposerPaxosState::FAILED;
        dependencies.logger->debugInfo("Failed to receive quorum of ACCEPT on key {0} and next {1}. Aborting", key.toString(), nextTimestamp.toString());
        return Response::error(ErrorCode::CAS_FAILED);
    }

    dependencies.logger->debugInfo("Received successfully quorum of ACCEPT on key {0} and next {1}. Saving to local db",
                                   key.toString(), nextTimestamp.toString());

    if(memDbStore->put(key, newValue, nextTimestamp, LamportClock::UpdateClockStrategy::SET_MAX, dependencies.clock, true).is_success()) {
        proposerPaxosRound->state = ProposerPaxosState::COMITTED;
        dependencies.operationLog->add(options.partitionId, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::SET)
            ->args({key, newValue})
            ->timestamp(nextTimestamp.counter)
            ->selfNode(nextTimestamp.nodeId)
            ->buildOperationBody());

        dependencies.logger->debugInfo("Successfully saved CAS key = {0} value = {1} with timestamp {2} into local db",
                                       key.toString(), newValue.toString(), nextTimestamp.toString());

        return Response::success();
    } else {
        proposerPaxosRound->state = ProposerPaxosState::FAILED;
        dependencies.logger->debugInfo("Failed to save CAS key = {0} value = {1} with timestamp {2} into local db",
                                       key.toString(), newValue.toString(), nextTimestamp.toString());

        return Response::error(ErrorCode::CAS_FAILED);
    }
}

std::result<std::tuple<LamportClock, LamportClock>> CasOperator::sendRetriesPrepares(OperatorDependencies& dependencies,
                                                                                     proposerPaxosRound_t proposerPaxosRound,
                                                                        int partitionId,
                                                                        SimpleString<memDbDataLength_t> key,
                                                                        SimpleString<memDbDataLength_t> expectedValue) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(partitionId);
    uint32_t keyHash = memDbStore->calculateHash(key);
    LamportClock prevTimestamp = {};
    LamportClock nextTimestamp = {};

    while(true){
        std::optional<MapEntry<memDbDataLength_t>> storedInDb = memDbStore->get(key);
        if(!storedInDb.has_value() || (storedInDb->value != expectedValue)) {
            proposerPaxosRound->state = ProposerPaxosState::FAILED;
            dependencies.logger->debugInfo("Expected value doest match with actual value stored in local db at key {0}", key.toString());
            return std::error<std::tuple<LamportClock, LamportClock>>();
        }

        prevTimestamp = storedInDb->timestamp;
        nextTimestamp = this->getNextTimestampForKey(storedInDb.value(), dependencies);

        dependencies.logger->debugInfo("Sending PREPARE(prev = {0}, next = {1}, key = {2}) to nodes in partition {3}",
                                       prevTimestamp.toString(), nextTimestamp.toString(), key.toString(), partitionId);

        multipleResponses_t prepareMultiResponses = sendPrepare(dependencies, proposerPaxosRound, partitionId, key, prevTimestamp, nextTimestamp);
        if(checkIfQuorumAndAllResponsesSuccess(prepareMultiResponses, proposerPaxosRound, dependencies)) {
            dependencies.logger->debugInfo("Received successfully quorum of promises on key {0} and next {1}", key.toString(), nextTimestamp.toString());
            break;
        }

        dependencies.logger->debugInfo("Promise quorum on key {0} and next {1} failed. Sleeping and retrying", key.toString(), nextTimestamp.toString());

        proposerPaxosRound->retryPrepareTimer.sleep();
    }

    proposerPaxosRound->retryPrepareTimer.reset();

    return std::ok<std::tuple<LamportClock, LamportClock>>({prevTimestamp, nextTimestamp});
}

bool CasOperator::checkIfQuorumAndAllResponsesSuccess(multipleResponses_t multiResponses, proposerPaxosRound_t proposerPaxosRound, OperatorDependencies& dependencies) {
    if(!multiResponses->waitForSuccessfulQuorum(dependencies.configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS))) {
        proposerPaxosRound->state = ProposerPaxosState::FAILED;
        return false;
    }
    return true;
}

multipleResponses_t CasOperator::sendPrepare(OperatorDependencies& dependencies,
                                             proposerPaxosRound_t proposerPaxosRound,
                                             int partitionId,
                                             SimpleString<memDbDataLength_t> key,
                                             LamportClock prevTimestamp,
                                             LamportClock nextTimestamp) {
    proposerPaxosRound->state = ProposerPaxosState::WAITING_FOR_PROMISE;

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
                                            proposerPaxosRound_t proposerPaxosRound,
                                            int partitionId,
                                            SimpleString<memDbDataLength_t> key,
                                            SimpleString<memDbDataLength_t> value,
                                            LamportClock prevTimestamp,
                                            LamportClock nextTimestamp) {
    proposerPaxosRound->state = ProposerPaxosState::WAITING_FOR_ACCEPT;

    dependencies.logger->debugInfo("Sending ACCEPT(key = {0}, value = {1}, prev = {2}, next = {3})", key.toString(),
                                   value.toString(), prevTimestamp.toString(), nextTimestamp.toString());

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

std::tuple<SimpleString<memDbDataLength_t>, SimpleString<memDbDataLength_t>, SimpleString<memDbDataLength_t>> CasOperator::getArgs(const OperationBody& operation) {
    return std::make_tuple(operation.getArg(0), operation.getArg(1), operation.getArg(2));
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
}´