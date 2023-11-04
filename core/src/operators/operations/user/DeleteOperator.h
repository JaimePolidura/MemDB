#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ResponseBuilder.h"

class DeleteOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    LamportClock getTimestamp(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies);
};