#include "AcceptCastOperator.h"

Response AcceptCastOperator::operate(const OperationBody &operation, const OperationOptions options, OperatorDependencies &dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);

    SimpleString<memDbDataLength_t> key = operation.getArg(0);
    SimpleString<memDbDataLength_t> expectedValue = operation.getArg(1);
    SimpleString<memDbDataLength_t> newValue = operation.getArg(2);
    uint32_t keyHash = memDbStore->calculateHash(key);
    LamportClock proposerTimestamp = this->getTimestampFromArgs(operation);

    std::optional<MapEntry<memDbDataLength_t>> keyInDb = memDbStore->get(key);
    std::optional<PaxosRound> paxosRound = dependencies.onGoingPaxosRounds->getRoundByKeyHash(keyHash);

    bool moreUpToDateValueIsStored = keyInDb.has_value() && keyInDb->timestamp.counter > proposerTimestamp.counter;
    bool promisedHigherTimestamp = paxosRound.has_value() && paxosRound->promisedTimestamp > proposerTimestamp;

    if(moreUpToDateValueIsStored || promisedHigherTimestamp){
        return Response::error(ErrorCode::CAS_FAILED);
    }
    if(!paxosRound.has_value()){
        dependencies.onGoingPaxosRounds->registerNewPaxosRoundAccept(keyHash, proposerTimestamp);
    }

    return Response::success();
}

OperatorDescriptor AcceptCastOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::CAS_ACCEPT,
            .name = "CAS_ACCEPT",
            .authorizedToExecute = { AuthenticationType::API },
    };
}

LamportClock AcceptCastOperator::getTimestampFromArgs(const OperationBody& operation) {
    uint16_t lamportNodeId = operation.getArg(3).to<uint16_t>();
    uint64_t lamportCounter = operation.getDoubleArgU64(4);

    return LamportClock{lamportNodeId, lamportCounter};
}