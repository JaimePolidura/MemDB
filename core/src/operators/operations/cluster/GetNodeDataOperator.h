#pragma once

#include "operators/Operator.h"
#include "messages/response/ResponseBuilder.h"
#include "messages/response/ErrorCode.h"

class GetNodeDataOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    SimpleString<memDbDataLength_t> nodeToSimpleString(node_t node);
};
