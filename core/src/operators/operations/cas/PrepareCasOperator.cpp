#include "PrepareCasOperator.h"

Response PrepareCasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    auto[key, prevTimestamp, nextTimestamp] = this->getArgs(operation);
    uint32_t keyHash = memDbStore->calculateHash(key);

    std::optional<MapEntry<memDbDataLength_t>> keyInDb = memDbStore->get(key);
    std::optional<AcceptatorPaxosRound> paxosRound = dependencies.onGoingPaxosRounds->getAcceptatorByKeyHash(keyHash);
    if(keyInDb->type != NodeType::DATA) {
        return Response::error(ErrorCode::INVALID_TYPE);
    }
    bool moreUpToDateValueIsStored = keyInDb.has_value() && keyInDb->toData()->timestamp > prevTimestamp;
    bool promisedHigherTimestamp = paxosRound.has_value() && paxosRound->promisedNextTimestamp > nextTimestamp;

    dependencies.logger->debugInfo("Received PREPARE(prev = {0}, next = {1}, key = {2}) from node {3}. More up to date value stored {4}? {5} Promised higher next timestamp {6}? {7}",
                                   prevTimestamp.toString(), nextTimestamp.toString(), key.toString(), std::to_string(operation.nodeId), keyInDb->toData()->timestamp.toString(),
                                   moreUpToDateValueIsStored, paxosRound->promisedNextTimestamp.toString(), promisedHigherTimestamp);
    
    if(moreUpToDateValueIsStored || promisedHigherTimestamp) {
        return Response::error(ErrorCode::CAS_FAILED);
    }
    if(!paxosRound.has_value()) {
        dependencies.onGoingPaxosRounds->registerNewAcceptatorPromise(keyHash, nextTimestamp);
    }
    if(paxosRound.has_value() && nextTimestamp > paxosRound->promisedNextTimestamp) {
        dependencies.onGoingPaxosRounds->updatePromisedTimestamp(keyHash, nextTimestamp);
    }

    return Response::success();
}

std::tuple<SimpleString<memDbDataLength_t>, LamportClock, LamportClock> PrepareCasOperator::getArgs(const OperationBody& operation) {
    SimpleString<memDbDataLength_t> key = operation.getArg(0);
    LamportClock prevTimestamp{operation.getArg(1).to<uint16_t>(), operation.getDoubleArgU64(2)};
    LamportClock nextTimestamp{operation.getArg(4).to<uint16_t>(), operation.getDoubleArgU64(5)};

    return std::make_tuple(key, prevTimestamp, nextTimestamp);
}

OperatorDescriptor PrepareCasOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::CAS_PREPARE,
            .name = "CAS_PREPARE",
            .authorizedToExecute = { AuthenticationType::NODE },
            .properties =  {}
    };
}