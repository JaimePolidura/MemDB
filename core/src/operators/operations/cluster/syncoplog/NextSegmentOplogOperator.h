#pragma once

#include "shared.h"
#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ErrorCode.h"
#include "messages/response/ResponseBuilder.h"

class NextSegmentOplogOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    oplogSegmentIterator_t getOplogSegmentIterator(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies);
};
