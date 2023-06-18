#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "persistence/OperationLogDeserializer.h"
#include "persistence/OperationLogInvalidator.h"
#include "persistence/OperationLogUtils.h"

/**
 * Flags:
 * - flag1 bool applyNewOplog
 * - flag2 bool clearOldOplog
 *
 * Args:
 *  - uint32_t newOplogId
 *  - uint32_t oldOplogId
 *  - std::vector<uint8_t> newOperationLog
 */
class MovePartitionOplogOperator : public Operator {
private:
    OperationLogDeserializer operationLogDeserializer;
    OperationLogInvalidator operationLogInvalidator;

public:
    static constexpr int OPERATOR_NUMBER = 0x06;

    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies dependencies) override {
        bool applyNewOplog = operation.flag1;
        bool clearOldOplog = operation.flag2;
        uint32_t newOplogId = operation.getArg(0).to<uint32_t>();
        uint32_t oldOplogId = operation.getArg(1).to<uint32_t>();

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

    OperatorType type() override {
        return OperatorType::NODE_MAINTENANCE;
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::NODE };
    }

    std::vector<OperatorDependency> dependencies() override {
        return { OperatorDependency::OPERATION_LOG, OperatorDependency::OPERATOR_DISPATCHER, OperatorDependency::CLUSTER };
    }

    std::string name() override {
        return "MOVE_PARTITION_OPLOG";
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

private:
    void clearOperationLog(OperatorDependencies dependencies, uint32_t operationLogId) {
        std::vector<OperationBody> operationsCleared = dependencies.operationLog->clear(OperationLogOptions{.operationLogId = operationLogId});
        std::vector<OperationBody> invalidationOperations = this->operationLogInvalidator.getInvalidationOperations(operationsCleared);
        dependencies.operatorsDispatcher(invalidationOperations, {.checkTimestamps = false, .onlyExecute = true});
    }
};