#pragma once

#include "operators/Operator.h"
#include "messages/response/ResponseBuilder.h"

class FixOplogSegmentOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    std::pair<std::vector<uint8_t>, uint32_t> oplogIteratorToBytes(oplogIterator_t oplogIterator);
};
