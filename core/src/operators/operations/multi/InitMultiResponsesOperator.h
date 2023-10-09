#pragma once

#include "operators/Operator.h"

class InitMultiResponsesOperator : public Operator {
    static constexpr const uint8_t OPERATOR_NUMBER = 0x06;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    constexpr OperatorDescriptor desc() override;
};