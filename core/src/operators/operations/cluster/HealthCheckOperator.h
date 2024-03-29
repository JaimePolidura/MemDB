#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/OperatorNumbers.h"

class HealthCheckOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;
};