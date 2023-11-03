#include "operators/operations/cluster/MovePartitionOplogOperator.h"

Response MovePartitionOplogOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    std::lock_guard<std::mutex> guard(this->moveOplogMutex);

    bool applyNewOplog = operation.flag1;
    bool clearOldOplog = operation.flag2;
    auto newOplogId = operation.getArg(0).to<memdbOplogId_t>();
    auto oldOplogId = operation.getArg(1).to<memdbOplogId_t>();

    if(newOplogId >= dependencies.cluster->getPartitionObject()->getNodesPerPartition()) {
        this->clearOperationLog(dependencies, oldOplogId);
        return Response::success();
    }

    std::vector<uint8_t> oplogRaw =  operation.getArgOr(2, SimpleString<memDbDataLength_t>::fromVector({})).toVector();
    std::vector<OperationBody> oplog = this->operationLogDeserializer.deserializeAll(oplogRaw);

    if(applyNewOplog || !dependencies.operationLog->hasOplogFile(newOplogId)) {
        dependencies.logger->debugInfo("MovePartitionOplogOperator Applying {0} entries from oplog id {1}", oplog.size(), newOplogId);
        dependencies.operatorsDispatcher(oplog, {.checkTimestamps = true, .onlyExecute = true, .partitionId = newOplogId});
    }

    if(clearOldOplog){
        dependencies.logger->debugInfo("MovePartitionOplogOperator Clearing oplog {0}", oldOplogId);
        this->clearOperationLog(dependencies, oldOplogId);
    }

    dependencies.logger->debugInfo("MovePartitionOplogOperator Adding {0} oplog entries to oplog id {1} ", oplog.size(), newOplogId);

    dependencies.operationLog->addAll(newOplogId, oplog);

    dependencies.cluster->setRunning();

    return Response::success();
}

OperatorDescriptor MovePartitionOplogOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::MOVE_OPLOG,
            .name = "MOVE_PARTITION_OPLOG",
            .authorizedToExecute = { AuthenticationType::NODE },
    };
}

void MovePartitionOplogOperator::clearOperationLog(OperatorDependencies dependencies, memdbOplogId_t operationLogId) {
    if(operationLogId > 0) {
        dependencies.operationLog->clear(operationLogId);
        dependencies.memDbStores->removeByPartitionId(operationLogId);
    }
}