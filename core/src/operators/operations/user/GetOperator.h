#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

class GetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x02;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    constexpr OperatorDescriptor desc() override;
};