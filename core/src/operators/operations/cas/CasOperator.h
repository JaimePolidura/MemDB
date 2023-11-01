#pragma once

#include "operators/Operator.h"
#include "messages/response/ResponseBuilder.h"
#include "messages/response/ErrorCode.h"
#include "operators/operations/cas/OnGoingPaxos.h"

/**
 * Args:
 *  - uint32 Key
 *  - uint32 Expected value
 *  - uint32 New value
 */
class CasOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    bool promiseResponsesHasGreaterTimestamp(const LamportClock& clock, const std::map<memdbNodeId_t, Response>& prepareResponses);
};