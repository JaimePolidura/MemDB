#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

class HealthCheckOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x04;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override {
        return Response::success();
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::MAINTENANCE };
    }

    std::vector<OperatorDependency> dependencies() override {
        return {};
    }

    constexpr OperatorType type() override {
        return OperatorType::NODE_MAINTENANCE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

    std::string name() override {
        return "HEALTH_CHECK";
    }
};