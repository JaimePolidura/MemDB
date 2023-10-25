#pragma once

#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ErrorCode.h"
#include "utils/Utils.h"
#include "persistence/serializers/OperationLogSerializer.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "config/keys/ConfigurationKeys.h"
#include "operators/OperatorDependencies.h"
#include "messages/response/ResponseBuilder.h"

/**
 * Invoked by InitMultiResponse
 * Args:
 * - Operator number of syncOplog
 * - Timestamp to sync (part1)
 * - Timestamp to sync (part2)
 * - OpLog id to sync of self node
 */
class SyncOplogOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;
    
    OperatorDescriptor desc() override;
};