#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/MaintenanceOperatorExecutor.h"

class HealthCheckOperator : public Operator, public MaintenanceOperatorExecutor {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x04;

    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, operationLog_t operationLog) override {
        return Response::success();
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::MAINTENANCE };
    }

    constexpr OperatorType type() override {
        return OperatorType::CONTROL;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

    std::string name() override {
        return "HEALTH_CHECK";
    }
};