#include "FixOplogSegmentOperator.h"

Response FixOplogSegmentOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    uint32_t oplogId = operation.getArg(0).to<uint32_t>();
    uint64_t minTimestamp = operation.getDoubleArgU64(1);
    uint64_t maxTimestamp = operation.getDoubleArgU64(3);

    oplogIterator_t oplogIteratorResult = std::dynamic_pointer_cast<OplogIterator>(dependencies.operationLog->getBetweenTimestamps(minTimestamp, maxTimestamp, {
        .operationLogId = oplogId,
        .compressed = true,
    }));

    auto[bytesToReturn, uncompressedSize, successNotCorrupted] = this->oplogIteratorToBytes(oplogIteratorResult);

    dependencies.logger->debugInfo("Responding {0} to FIX_OPLOG_SEGMENT(oplogId = {1}, minTimestamp = {2}, maxTimestamp = {3}) with {4} bytes and uncompressed size: {5}",
                                   successNotCorrupted ? "successfully" : "unsuccessfully", oplogId, minTimestamp, maxTimestamp, bytesToReturn.size(), uncompressedSize);

    return ResponseBuilder::builder()
        .isSuccessful(successNotCorrupted, ErrorCode::UNFIXABLE_CORRUPTED_OPLOG_SEGMENT)
        ->values({
            SimpleString<memDbDataLength_t>::fromNumber(uncompressedSize),
            SimpleString<memDbDataLength_t>::fromVector(bytesToReturn),
        })
        ->build();
}

OperatorDescriptor FixOplogSegmentOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::FIX_OPLOG_SEGMENT,
            .name = "FIX_OPLOG_SEGMENT",
            .authorizedToExecute = { AuthenticationType::NODE }
    };
}

std::tuple<std::vector<uint8_t>, uint32_t, bool> FixOplogSegmentOperator::oplogIteratorToBytes(oplogIterator_t oplogIterator) {
    std::vector<uint8_t> result{};
    uint32_t uncompressedSize = 0;

    while(oplogIterator->hasNext()){
        uncompressedSize += oplogIterator->getNextUncompressedSize();
        std::result<std::vector<uint8_t>> bytesResult = oplogIterator->next();

        if(bytesResult.has_error()){
            return std::make_tuple(result, uncompressedSize, false);
        }

        std::vector<uint8_t> bytes = bytesResult.get();
        result.insert(result.begin(), bytes.begin(), bytes.end());
    }

    return std::make_tuple(result, uncompressedSize, true);
}