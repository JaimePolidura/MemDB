#include "FixOplogSegmentOperator.h"

Response FixOplogSegmentOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    uint32_t oplogId = operation.getArg(0).to<uint32_t>();
    uint64_t minTimestamp = operation.getDoubleArgU64(1);
    uint64_t maxTimestamp = operation.getDoubleArgU64(3);

    oplogIterator_t oplogIteratorResult = std::dynamic_pointer_cast<OplogIterator>(dependencies.operationLog->getBetweenTimestamps(minTimestamp, maxTimestamp, {
        .operationLogId = oplogId,
        .compressed = true,
    }));

    auto[bytesToReturn, uncompressedSize] = this->oplogIteratorToBytes(oplogIteratorResult);

    return ResponseBuilder::builder()
        .success()
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

std::pair<std::vector<uint8_t>, uint32_t> FixOplogSegmentOperator::oplogIteratorToBytes(oplogIterator_t oplogIterator) {
    std::vector<uint8_t> result(oplogIterator->totalSize());
    uint32_t uncompressedSize = 0;

    while(oplogIterator->hasNext()){
        uncompressedSize += oplogIterator->getNextUncompressedSize();

        std::vector<uint8_t> bytes = oplogIterator->next();
        result.insert(result.begin(), bytes.begin(), bytes.end());
    }

    return std::make_pair(result, uncompressedSize);
}