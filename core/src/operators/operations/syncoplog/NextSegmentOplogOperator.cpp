#include "NextSegmentOplogOperator.h"

Response NextSegmentOplogOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    oplogIterator_t oplogSenderIterator = this->getOplogSegmentIterator(operation, options, dependencies);
    uint64_t syncId = options.requestNumber;

    if(!oplogSenderIterator->hasNext()) {
        dependencies.onGoingSyncOplogs->removeBySyncId(syncId);
        return Response::error(ErrorCode::SYNC_OP_LOG_EOF);
    }

    dependencies.onGoingSyncOplogs->markSegmentAsSent(syncId);

    auto[compressedOplog, uncompressSize, success] = this->getNextOplogSegmentOrTryFix(oplogSenderIterator, dependencies);

    return ResponseBuilder::builder()
            .timestampCounter(oplogSenderIterator->getLastTimestampOfLastNext())
            ->isSuccessful(success, ErrorCode::UNFIXABLE_CORRUPTED_OPLOG_SEGMENT)
            ->requestNumber(syncId)
            ->values({
                SimpleString<memDbDataLength_t>::fromNumber(uncompressSize),
                SimpleString<memDbDataLength_t>::fromVector(compressedOplog)
            })
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

std::tuple<std::vector<uint8_t>, uint32_t, bool> NextSegmentOplogOperator::getNextOplogSegmentOrTryFix(oplogIterator_t iterator, OperatorDependencies& dependencies) {
    std::result<std::vector<uint8_t>> bytesFromOplogIteratorResult = iterator->next();

    if(bytesFromOplogIteratorResult.is_success()){
        return std::make_tuple(bytesFromOplogIteratorResult.get(), iterator->getLastUncompressedSize(), true);
    }

    OplogIndexSegmentDescriptor corruptedDescriptor = iterator->getLastDescriptor();

    std::result<Response> fixOplogResponse =
            dependencies.cluster->fixOplogSegment(iterator->getOplogId(), corruptedDescriptor.min, corruptedDescriptor.max);

    if(fixOplogResponse.has_error() || !fixOplogResponse->isSuccessful) { //No node available or all nodes have corrupted oplog
        return std::make_tuple(std::vector<uint8_t>{}, 0, false);
    }

    uint32_t uncompressedSize = fixOplogResponse->getResponseValueAtOffset(0, sizeof(uint32_t)).to<uint32_t>();
    std::vector<uint8_t> uncorruptedBytes = fixOplogResponse->getResponseValueAtOffset(sizeof(uint32_t),
        fixOplogResponse->responseValue.size - sizeof(uint32_t)).toVector();

    dependencies.oplogIndexSegment->updateCorruptedSegment(uncorruptedBytes, uncompressedSize, iterator->getLastSegmentOplogDescriptorDiskPtr());

    return std::make_tuple(uncorruptedBytes, uncompressedSize, true);
}

oplogIterator_t NextSegmentOplogOperator::getOplogSegmentIterator(const OperationBody &operation, const OperationOptions options,
                                                                  OperatorDependencies &dependencies) {
    std::optional<oplogIterator_t> senderIteratorOptional = dependencies.onGoingSyncOplogs->getSenderIteratorById(options.requestNumber);

    if(!senderIteratorOptional.has_value()) {
        dependencies.logger->debugInfo("Received a NEXT_SYNC_OPLOG_SEGMENT at requestNumber {0} without having an iterator registered. Restarting SYNC_OPLOG", options.requestNumber);

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