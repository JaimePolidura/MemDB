#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

class HealthCheckOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x04;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override;

    std::vector<AuthenticationType> authorizedToExecute() override;

    std::vector<OperatorDependency> dependencies() override;

    constexpr OperatorType type() override;

    constexpr uint8_t operatorNumber() override;

    std::string name() override;
};