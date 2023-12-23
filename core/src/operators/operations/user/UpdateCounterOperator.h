#pragma once

#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ErrorCode.h"
#include "messages/response/ResponseBuilder.h"

/**
 * Flags:
 * - flag1 bool increment
 *
 * Args:
 *  - string key
 */
class UpdateCounterOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;

private:
    void replicate(Counter& counter, uint64_t newUpdatedValue, OperatorDependencies& dependencies, const OperationBody& operationBody);

    void onReplicationCounterResponse(const SimpleString<memDbDataLength_t>& key, const Response& response, Counter& counter);

    uint32_t getNNodesInCluster(OperatorDependencies&dependencies);
};
