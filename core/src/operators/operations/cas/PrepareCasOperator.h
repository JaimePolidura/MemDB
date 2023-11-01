#pragma once

#include "operators/Operator.h"
#include "messages/response/ResponseBuilder.h"
#include "messages/response/ErrorCode.h"

/**
 * Args:
 *  - uint32 Key
 *  - uint32 Expected Value
 *  - uint32 New Value
 *  - uint32 NodeId (lamport's nodeId of proposer) Read as uint16
 *  - uint64 Counter (lamport's nodeId of proposer)
 */
class PrepareCasOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    LamportClock getTimestampFromArgs(const OperationBody& operation);
};
