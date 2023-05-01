#pragma once

#include "operators/Operator.h"
#include "operators/MaintenanceOperatorExecutor.h"
#include "messages/response/ErrorCode.h"

class MovePartitionOplogOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x05;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override {

    }

    std::vector<OperatorDependency> dependencies() override {
        return { OperatorDependency::DB_STORE, OperatorDependency::CLUSTER, OperatorDependency::OPERATION_LOG };
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
        return "MOVE_PARTITION_OPLOG";
    }
};