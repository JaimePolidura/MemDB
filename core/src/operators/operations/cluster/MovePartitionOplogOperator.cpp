#include "operators/operations/cluster/MovePartitionOplogOperator.h"

Response MovePartitionOplogOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    bool applyNewOplog = operation.flag1;
    bool clearOldOplog = operation.flag2;
    auto newOplogId = operation.getArg(0).to<uint32_t>();
    auto oldOplogId = operation.getArg(1).to<uint32_t>();

    //Oplogs ids start with 0. 0 = self node
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
            .dontUseBuffer = true,
    });

    dependencies.cluster->setRunning();

    return Response::success();
}

OperatorType MovePartitionOplogOperator::type() {
    return OperatorType::NODE_MAINTENANCE;
}

std::vector<AuthenticationType> MovePartitionOplogOperator::authorizedToExecute() {
    return { AuthenticationType::NODE };
}

std::string MovePartitionOplogOperator::name() {
    return "MOVE_PARTITION_OPLOG";
}

constexpr uint8_t MovePartitionOplogOperator::operatorNumber() {
    return OPERATOR_NUMBER;
}

void MovePartitionOplogOperator::clearOperationLog(OperatorDependencies dependencies, uint32_t operationLogId) {
    std::vector<OperationBody> operationsCleared = dependencies.operationLog->clear(OperationLogOptions{.operationLogId = operationLogId});
    std::vector<OperationBody> invalidationOperations = this->operationLogInvalidator.getInvalidationOperations(operationsCleared);
    dependencies.operatorsDispatcher(invalidationOperations, {.checkTimestamps = false, .onlyExecute = true});
}