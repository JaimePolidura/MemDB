#pragma once

#include "shared.h"
#include "operators/Operator.h"

class NextFragmentOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x07;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    constexpr OperatorDescriptor desc() override;
};
