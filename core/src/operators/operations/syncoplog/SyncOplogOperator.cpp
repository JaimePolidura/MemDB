#include "SyncOplogOperator.h"

Response SyncOplogOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    uint64_t lastTimestampUnsync = operation.getDoubleArgU64(0);
    uint32_t nodeOplogIdToSync = operation.getArg(2).to<uint32_t>();
    uint64_t syncId = options.requestNumber;

    oplogIterator_t segmentIterator = std::dynamic_pointer_cast<OplogIterator>(
            dependencies.operationLog->getAfterTimestamp(lastTimestampUnsync, OperationLogOptions{
                .operationLogId = nodeOplogIdToSync,
                .compressed = true,
            })
    );

    dependencies.onGoingSyncOplogs->registerSyncOplogIterator(syncId, segmentIterator);

    dependencies.logger->debugInfo("Responding to SYNC_OPLOG from node {0} of timestamp to sync {1} syncId: {2} and selfOplogId {3} with an iterator with {4} entries",
                                   operation.nodeId, syncId, lastTimestampUnsync, nodeOplogIdToSync, segmentIterator->totalSize());

    return ResponseBuilder::builder()
        .value(SimpleString<memDbDataLength_t>::fromNumber(segmentIterator->totalSize()))
        ->requestNumber(syncId)
        ->success()
        ->build();
}

OperatorDescriptor SyncOplogOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::SYNC_OPLOG,
            .name = "SYNC_OPLOG",
            .authorizedToExecute = { AuthenticationType::NODE },
            .properties = {}
    };
}