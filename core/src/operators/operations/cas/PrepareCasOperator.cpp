#include "PrepareCasOperator.h"

Response PrepareCasOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);

    SimpleString<memDbDataLength_t> key = operation.getArg(0);
    SimpleString<memDbDataLength_t> expectedValue = operation.getArg(1);
    SimpleString<memDbDataLength_t> newValue = operation.getArg(2);
    LamportClock timestampFromPrepare = this->getTimestampFromArgs(operation);
    uint32_t keyHash = memDbStore->calculateHash(key);

    std::optional<MapEntry<memDbDataLength_t>> keyInDb = memDbStore->get(key);
    std::optional<PaxosRound> paxosRound = dependencies.onGoingPaxosRounds->getRoundByKeyHash(keyHash);
    bool moreUpToDateValueIsStored = keyInDb.has_value() && keyInDb->timestamp.counter > timestampFromPrepare.counter;
    bool promisedHigherTimestamp = paxosRound.has_value() && paxosRound->promisedTimestamp > timestampFromPrepare;

    if(moreUpToDateValueIsStored || promisedHigherTimestamp) {
        return Response::error(ErrorCode::CAS_FAILED);
    }
    if(!paxosRound.has_value()) {
        dependencies.onGoingPaxosRounds->registerNewPaxosRoundPromised(keyHash, timestampFromPrepare);
    }
    if(paxosRound.has_value() && timestampFromPrepare > paxosRound->promisedTimestamp) {
        dependencies.onGoingPaxosRounds->updatePromisedTimestamp(keyHash, timestampFromPrepare);
    }

    if(paxosRound.has_value() && paxosRound->state == PaxosState::ACCEPTED){
        return ResponseBuilder::builder()
            .error(ErrorCode::CAS_PROMISE_ACCEPTED)
            ->values({

            })
            ->build();
    }

    return Response::success();
}

OperatorDescriptor PrepareCasOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::CAS_PREPARE,
            .name = "CAS_PREPARE",
            .authorizedToExecute = { AuthenticationType::NODE },
    };
}

LamportClock PrepareCasOperator::getTimestampFromArgs(const OperationBody& operation) {
    uint16_t lamportNodeId = operation.getArg(3).to<uint16_t>();
    uint64_t lamportCounter = operation.getDoubleArgU64(4);

    return LamportClock{lamportNodeId, lamportCounter};
}