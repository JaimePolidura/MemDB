#pragma once

#include "shared.h"
#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"

class NextFragmentOperator : public Operator {
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;
};
