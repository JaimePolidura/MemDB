#pragma once


#include "operators/Operator.h"
#include "operators/MaintenanceOperatorExecutor.h"
#include "messages/response/ErrorCode.h"
#include "persistence/OperationLogDeserializer.h"
#include "persistence/OperationLogUtils.h"

class MovePartitionOplogOperator : public Operator {
private:
    OperationLogDeserializer operationLogDeserializer;

public:
    static constexpr int OPERATOR_NUMBER = 0x07;

    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies dependencies) override {
        int newOplogId = operation.getArg(0).to<int>();
        int oldOplogId = newOplogId - 1;

        if(newOplogId > dependencies.cluster->getPartitionObject()->getNodesPerPartition()) {
            std::vector<OperationBody> operationsCleared = dependencies.operationLog->clear(OperationLogOptions{.operationLogId = newOplogId});
            this->dispatchOperations(operationsCleared, dependencies);
            return Response::success();
        }

        std::vector<uint8_t> oplogRaw = operation.getArg(1).toVector();
        std::vector<OperationBody> oplog = this->operationLogDeserializer.deserializeAll(oplogRaw);

        dependencies.operationLog->addAll(oplog, OperationLogOptions{
                .operationLogId = newOplogId,
                .dontUseBuffer = true,
        });

        dependencies.operationLog->clear(OperationLogOptions{.operationLogId = oldOplogId});

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

private:
    void dispatchOperations(const std::vector<OperationBody>& operations, OperatorDependencies dependencies) {
        for (const OperationBody& operation: operations)  {
            dependencies.operatorDispatcher(operation, OperationOptions{
                .onlyExecute = true,
            });
        }
    }
};