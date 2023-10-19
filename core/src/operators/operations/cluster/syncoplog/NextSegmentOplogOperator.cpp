#include "NextSegmentOplogOperator.h"

Response NextSegmentOplogOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    oplogSegmentIterator_t oplogSenderIterator = this->getOplogSegmentIterator(operation, options, dependencies);
    uint64_t syncId = options.requestNumber;

    if(!oplogSenderIterator->hasNext()) {
        dependencies.onGoingSyncOplogs->removeBySyncId(syncId);
        return Response::error(ErrorCode::SYNC_OP_LOG_EOF);
    }

    dependencies.onGoingSyncOplogs->markSegmentAsSent(syncId);

    return ResponseBuilder::builder()
            .value(SimpleString<memDbDataLength_t>::fromVector(oplogSenderIterator->next()))
            ->timestamp(oplogSenderIterator->getLastTimestampOfLastNext())
            ->requestNumber(syncId)
            ->success()
            ->build();
}

OperatorDescriptor NextSegmentOplogOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::NEXT_SYNC_OPLOG_SEGMENT,
        .name = "NEXT_SYNC_OPLOG_SEGMENT",
        .authorizedToExecute = { AuthenticationType::NODE },
    };
}

oplogSegmentIterator_t NextSegmentOplogOperator::getOplogSegmentIterator(const OperationBody &operation, const OperationOptions options,
                                                  OperatorDependencies &dependencies) {
    std::optional<oplogSegmentIterator_t> senderIteratorOptional = dependencies.onGoingSyncOplogs->getSenderIteratorById(options.requestNumber);

    if(!senderIteratorOptional.has_value()) {
        uint64_t lastTimestampUnsync = operation.getDoubleArgU64(0);
        uint32_t nodeOplogIdToSync = operation.getArg(2).to<uint32_t>();

        dependencies.operatorDispatcher(RequestBuilder::builder()
            .operatorNumber(OperatorNumbers::SYNC_OPLOG)
            ->args({
                SimpleString<memDbDataLength_t>::fromNumber((uint32_t) (lastTimestampUnsync >> 32)),
                SimpleString<memDbDataLength_t>::fromNumber((uint32_t) (lastTimestampUnsync & 0xFFFFFFFF)),
                SimpleString<memDbDataLength_t>::fromNumber(nodeOplogIdToSync)
            })
            ->buildOperationBody(), options);

        senderIteratorOptional = dependencies.onGoingSyncOplogs->getSenderIteratorById(options.requestNumber);
    }

    return senderIteratorOptional.value();
}