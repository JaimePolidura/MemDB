#include "AcceptCasOperator.h"

Response AcceptCasOperator::operate(const OperationBody &operation, const OperationOptions options, OperatorDependencies &dependencies) {
    memDbDataStoreMap_t memDbStore = dependencies.memDbStores->getByPartitionId(options.partitionId);
    auto[key, value, prevTimestamp, nextTimestamp] = this->getArgs(operation);
    uint32_t keyHash = memDbStore->calculateHash(key);

    std::optional<MapEntry<memDbDataLength_t>> keyInDb = memDbStore->get(key);
    std::optional<AcceptatorPaxosRound> paxosRound = dependencies.onGoingPaxosRounds->getAcceptatorRoundByKeyHash(keyHash);

    bool moreUpToDateValueIsStored = keyInDb.has_value() && keyInDb->timestamp > prevTimestamp;
    bool promisedHigherTimestamp = paxosRound.has_value() && paxosRound->acceptatorPromisedNextTimestamp > nextTimestamp;

    if(moreUpToDateValueIsStored || promisedHigherTimestamp){
        return Response::error(ErrorCode::CAS_FAILED);
    }

    bool success = memDbStore->put(key, value, false, nextTimestamp.counter, nextTimestamp.nodeId);

    if(success) {
        dependencies.onGoingPaxosRounds->updateAcceptedTimestamp(keyHash, nextTimestamp);
        dependencies.operationLog->add(options.partitionId, RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::SET)
            ->args({key, value})
            ->timestamp(nextTimestamp.counter)
            ->selfNode(nextTimestamp.nodeId)
            ->buildOperationBody());
    }

    return ResponseBuilder::builder()
        .isSuccessful(success, ErrorCode::CAS_FAILED)
        ->build();
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
            .authorizedToExecute = { AuthenticationType::API },
    };
}