#include "PrepareCasOperator.h"

Response PrepareCasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);

    auto[key, prevTimestamp, nextTimestamp] = this->getArgs(operation);

    uint32_t keyHash = memDbStore->calculateHash(key);

    std::optional<MapEntry<memDbDataLength_t>> keyInDb = memDbStore->get(key);
    std::optional<PaxosRound> paxosRound = dependencies.onGoingPaxosRounds->getRoundByKeyHash(keyHash);
    bool moreUpToDateValueIsStored = keyInDb.has_value() && keyInDb->timestamp > prevTimestamp;
    bool promisedHigherTimestamp = paxosRound.has_value() && paxosRound->promisedTimestamp > nextTimestamp;

    if(moreUpToDateValueIsStored || promisedHigherTimestamp) {
        return Response::error(ErrorCode::CAS_FAILED);
    }
    if(!paxosRound.has_value()) {
        dependencies.onGoingPaxosRounds->registerNewPaxosRoundPromised(keyHash, nextTimestamp);
    }
    if(paxosRound.has_value() && nextTimestamp > paxosRound->promisedTimestamp) {
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
    };
}}