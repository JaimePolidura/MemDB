#include "SyncOplogOperator.h"

Response SyncOplogOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    uint64_t lastTimestampUnsync = operation.getDoubleArgU64(0);
    uint32_t nodeOplogIdToSync = operation.getArg(1).to<uint32_t>();
    uint64_t syncOplogId = options.requestNumber;

    oplogSegmentIterator_t segmentIterator = std::dynamic_pointer_cast<OplogIterator>(
            dependencies.operationLog->getAfterTimestamp(lastTimestampUnsync, OperationLogOptions{
                .operationLogId = nodeOplogIdToSync
            })
    );

    dependencies.onGoingSyncOplogs->registerSyncOplogIterator(syncOplogId, segmentIterator);

    return ResponseBuilder::builder()
        .value(SimpleString<memDbDataLength_t>::fromNumber(segmentIterator->totalSize()))
        ->requestNumber(syncOplogId)
        ->success()
        ->build();
}

OperatorDescriptor SyncOplogOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::SYNC_OPLOG,
            .name = "SYNC_OPLOG",
            .authorizedToExecute = { AuthenticationType::NODE },
    };
}