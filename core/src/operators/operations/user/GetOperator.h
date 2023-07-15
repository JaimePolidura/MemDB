#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperatorExecutor.h"

class GetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x02;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override;

    std::vector<OperatorDependency> dependencies() override;

    std::vector<AuthenticationType> authorizedToExecute() override;

    constexpr OperatorType type() override;

    constexpr uint8_t operatorNumber() override;

    std::string name() override;
};