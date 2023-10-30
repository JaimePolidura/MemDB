#include "NextSegmentOplogOperator.h"

Response NextSegmentOplogOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    oplogIterator_t oplogSenderIterator = this->getOplogSegmentIterator(operation, options, dependencies);
    uint64_t syncId = options.requestNumber;

    if(!oplogSenderIterator->hasNext()) {
        dependencies.onGoingSyncOplogs->removeBySyncId(syncId);
        return Response::error(ErrorCode::SYNC_OP_LOG_EOF);
    }

    dependencies.onGoingSyncOplogs->markSegmentAsSent(syncId);

    auto [compressedOplog, uncompressSize] = this->getNextOplogSegmentOrTryFix(oplogSenderIterator, dependencies);

    return ResponseBuilder::builder()
            .values({
                SimpleString<memDbDataLength_t>::fromNumber(uncompressSize),
                SimpleString<memDbDataLength_t>::fromVector(compressedOplog),
            })
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

std::pair<std::vector<uint8_t>, uint32_t> NextSegmentOplogOperator::getNextOplogSegmentOrTryFix(oplogIterator_t iterator, OperatorDependencies& dependencies) {
    std::result<std::vector<uint8_t>> bytesFromOplogIteratorResult = iterator->next();

    if(bytesFromOplogIteratorResult.is_success()){
        return std::make_pair(bytesFromOplogIteratorResult.get(), iterator->getLastUncompressedSize());
    }

    OplogIndexSegmentDescriptor corruptedDescriptor = iterator->getLastDescriptor();

    std::optional<Response> fixOplogResponse =
            dependencies.cluster->fixOplogSegment(iterator->getOplogId(), corruptedDescriptor.min, corruptedDescriptor.max);

    if(!fixOplogResponse.has_value() || !fixOplogResponse->isSuccessful) { //No node available or all nodes have corrupted oplog
        return std::make_pair(std::vector<uint8_t>{}, 0);
    }

    uint32_t uncompressedSize = fixOplogResponse.value().getResponseValueAtOffset(0, sizeof(uint32_t)).to<uint32_t>();
    std::vector<uint8_t> fixedBytes = fixOplogResponse->getResponseValueAtOffset(sizeof(uint32_t),
        fixOplogResponse->responseValue.size - sizeof(uint32_t)).toVector();

    dependencies.oplogIndexSegment->updateCorruptedSegment(uncompressedSize, iterator->getLastSegmentOplogDescriptorDiskPtr(), fixedBytes);

    return std::make_pair(fixedBytes, uncompressedSize);
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