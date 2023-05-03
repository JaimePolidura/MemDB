#pragma once

#include "operators/Operator.h"
#include "operators/MaintenanceOperatorExecutor.h"
#include "messages/response/ErrorCode.h"
#include "persistence/OperationLogDeserializer.h"

class SetNewPartitionOplogOperator : public Operator {
private:
    OperationLogDeserializer operationLogDeserializer;

public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x06;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override {
        auto oplogNewId = operation.getArg(0).to<int>();
        auto newOplogEntriesRaw = operation.getArg(1).toVector();
        auto newOplogEntries = this->operationLogDeserializer.deserializeAll(newOplogEntriesRaw);

        dependencies.operationLog->replaceAll(newOplogEntries, OperationLogOptions{.operationLogId = oplogNewId});
        for (const auto& newOplog: newOplogEntries) {
            dependencies.operatorDispatcher(newOplog, OperationOptions{.onlyExecute = true});
        }

        dependencies.cluster->setRunning();

        return Response::success();
    }

    std::vector<OperatorDependency> dependencies() override {
        return { OperatorDependency::DB_STORE, OperatorDependency::CLUSTER, OperatorDependency::OPERATION_LOG, OperatorDependency::OPERATOR_DISPATCHER };
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::NODE };
    }

    constexpr OperatorType type() override {
        return OperatorType::WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

    std::string name() override {
        return "SET_NEW_PARTITION_OPLOG";
    }
};