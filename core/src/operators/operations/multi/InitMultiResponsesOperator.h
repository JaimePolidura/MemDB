#pragma once

#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"

class InitMultiResponsesOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    constexpr OperatorDescriptor desc() override;
};