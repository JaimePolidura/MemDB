#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "persistence/OperationLogDeserializer.h"
#include "persistence/OperationLogInvalidator.h"
#include "persistence/OperationLogUtils.h"

class MovePartitionOplogOperator : public Operator {
private:
    OperationLogDeserializer operationLogDeserializer;
    OperationLogInvalidator operationLogInvalidator;

public:
    static constexpr int OPERATOR_NUMBER = 0x06;

    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies dependencies) override {
        uint32_t newOplogId = operation.getArg(0).to<uint32_t>();
        uint32_t oldOplogId = newOplogId - 1;

        //Oplogs ids start with 0. 0 = self node
        if(newOplogId + 1 >= dependencies.cluster->getPartitionObject()->getNodesPerPartition()) {
            std::vector<OperationBody> operationsCleared = dependencies.operationLog->clear(OperationLogOptions{.operationLogId = newOplogId});
            std::vector<OperationBody> invalidationOperations = this->operationLogInvalidator.getInvalidationOperations(operationsCleared);
            dependencies.operatorsDispatcher(invalidationOperations, {.onlyExecute = true});

            return Response::success();
        }

        std::vector<uint8_t> oplogRaw = operation.getArg(1).toVector();
        std::vector<OperationBody> oplog = this->operationLogDeserializer.deserializeAll(oplogRaw);

        //Doest have data stored
        if(!dependencies.operationLog->hasOplogFile({.operationLogId = newOplogId})){
            dependencies.operatorsDispatcher(oplog, {.onlyExecute = true});
        }

        dependencies.operationLog->addAll(oplog, OperationLogOptions{
                .operationLogId = newOplogId,
                .dontUseBuffer = true,
        });

        dependencies.operationLog->clear(OperationLogOptions{.operationLogId = oldOplogId});

        dependencies.cluster->setRunning();

        return Response::success();
    }

    OperatorType type() override {
        return OperatorType::CONTROL;
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
};