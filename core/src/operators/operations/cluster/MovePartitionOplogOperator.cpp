#include "operators/operations/cluster/MovePartitionOplogOperator.h"

Response MovePartitionOplogOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    bool applyNewOplog = operation.flag1;
    bool clearOldOplog = operation.flag2;
    auto newOplogId = operation.getArg(0).to<uint32_t>();
    auto oldOplogId = operation.getArg(1).to<uint32_t>();

    if(newOplogId >= dependencies.cluster->getPartitionObject()->getNodesPerPartition()) {
        this->clearOperationLog(dependencies, newOplogId);
        return Response::success();
    }

    std::vector<uint8_t> oplogRaw = operation.getArg(2).toVector();
    std::vector<OperationBody> oplog = this->operationLogDeserializer.deserializeAll(oplogRaw);

    //Doest have data stored
    if(applyNewOplog || !dependencies.operationLog->hasOplogFile({.operationLogId = newOplogId})) {
        dependencies.operatorsDispatcher(oplog, {.checkTimestamps = true, .onlyExecute = true});
    }

    if(clearOldOplog){
        this->clearOperationLog(dependencies, oldOplogId);
    }

    dependencies.operationLog->addAll(oplog, OperationLogOptions{
            .operationLogId = newOplogId,
    });

    dependencies.cluster->setRunning();

    return Response::success();
}

constexpr OperatorDescriptor MovePartitionOplogOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::MOVE_OPLOG,
            .name = "MOVE_PARTITION_OPLOG",
            .authorizedToExecute = { AuthenticationType::NODE },
    };
}

void MovePartitionOplogOperator::clearOperationLog(OperatorDependencies dependencies, uint32_t operationLogId) {
    if(operationLogId > 0) {
        dependencies.operationLog->clear(OperationLogOptions{.operationLogId = operationLogId});
        dependencies.memDbStores->removeByPartitionId(operationLogId);
    }
}