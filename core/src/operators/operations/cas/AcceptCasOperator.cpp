#include "AcceptCasOperator.h"

Response AcceptCasOperator::operate(const OperationBody &operation, const OperationOptions options, OperatorDependencies &dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    auto[key, value, prevTimestamp, nextTimestamp] = this->getArgs(operation);
    uint32_t keyHash = memDbStore->calculateHash(key);

    std::optional<MapEntry<memDbDataLength_t>> keyInDb = memDbStore->get(key);
    std::optional<AcceptatorPaxosRound> paxosRound = dependencies.onGoingPaxosRounds->getAcceptatorByKeyHash(keyHash);

    if(keyInDb->type != NodeType::DATA) {
        return Response::error(ErrorCode::INVALID_TYPE);
    }

    bool moreUpToDateValueIsStored = keyInDb.has_value() && keyInDb->toData()->timestamp > prevTimestamp;
    bool promisedHigherTimestamp = paxosRound.has_value() && paxosRound->promisedNextTimestamp > nextTimestamp;

    dependencies.logger->debugInfo("Received ACCEPT(prev = {0}, next = {1}, key = {2}, value = {3}) from node {4}. More up to date value stored {5}? {6} Promised higher next timestamp {7}? {8}",
                                   prevTimestamp.toString(), nextTimestamp.toString(), key.toString(), std::to_string(operation.nodeId), value.toString(),
                                   keyInDb->toData()->timestamp.toString(), moreUpToDateValueIsStored, paxosRound->promisedNextTimestamp.toString(), promisedHigherTimestamp);

    if(moreUpToDateValueIsStored || promisedHigherTimestamp){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    if(memDbStore->putData(key, value, nextTimestamp, LamportClock::UpdateClockStrategy::SET_MAX, dependencies.clock, true).is_success()) {
        dependencies.logger->debugInfo("Successfully saved CAS key = {0} value = {1} with timestamp {2} into local db",
                                       key.toString(), value.toString(), nextTimestamp.toString());

        dependencies.onGoingPaxosRounds->updateAcceptedTimestamp(keyHash, nextTimestamp);
        dependencies.operationLog->add(options.partitionId, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::SET)
            ->args({key, value})
            ->timestamp(nextTimestamp.counter)
            ->selfNode(nextTimestamp.nodeId)
            ->buildOperationBody());

        return Response::success();
    } else {
        dependencies.logger->debugInfo("Failed to save CAS key = {0} value = {1} with timestamp {2} into local db",
                                       key.toString(), value.toString(), nextTimestamp.toString());

        return Response::error(ErrorCode::CAS_FAILED);
    }
}

std::tuple<SimpleString<memDbDataLength_t>, SimpleString<memDbDataLength_t>, LamportClock, LamportClock> AcceptCasOperator::getArgs(const OperationBody& operation) {
    SimpleString<memDbDataLength_t> key = operation.getArg(0);
    SimpleString<memDbDataLength_t> value = operation.getArg(1);
    LamportClock prevTimestamp{operation.getArg(2).to<uint16_t>(), operation.getDoubleArgU64(3)};
    LamportClock nextTimestamp{operation.getArg(5).to<uint16_t>(), operation.getDoubleArgU64(6)};

    return std::make_tuple(key, value, prevTimestamp, nextTimestamp);
}

OperatorDescriptor AcceptCasOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::CAS_ACCEPT,
            .name = "CAS_ACCEPT",
            .authorizedToExecute = { AuthenticationType::NODE },
            .properties =  {}
    };
}