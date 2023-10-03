#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperatorExecutor.h"

class DeleteOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x03;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override;

    std::vector<AuthenticationType> authorizedToExecute() override;

    std::vector<OperatorDependency> dependencies() override;

    OperatorType type() override;

    uint8_t operatorNumber() override;

    std::string name() override;
};