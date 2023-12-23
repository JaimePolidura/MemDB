#pragma once

#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ErrorCode.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/utils/OperationLogInvalidator.h"
#include "persistence/utils/OperationLogUtils.h"

/**
 * Request:
 * Flags:
 * - flag1 bool increment
 *
 * Args:
 * - key
 * - uint64 newValue
 * - uint64 lastSeenIncrement
 * - uint64 lastSeenDecrement
 *
 * Response:
 * - uint64 newSyncIncrement if zero -> no need to sync
 * - uint64 newSyncDecrement if zero -> no need to sync
 * - memdbnodeId selfNodeId
 */
class ReplicateCounterOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;
};
