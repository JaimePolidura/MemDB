#pragma once

#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ErrorCode.h"
#include "utils/Utils.h"
#include "persistence/serializers/OperationLogSerializer.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "config/keys/ConfigurationKeys.h"
#include "operators/OperatorDependencies.h"

/**
 * Invoked by InitMultiResponse
 * Args:
 * - Operator number of syncOplog
 * - Timestamp to sync
 * - Timestamp to sync
 * - Node id: nodeOplogIdToSync
 */
class SyncOplogOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    iterator_t<std::vector<uint8_t>> createMultiResponseSenderIterator(const OperationBody& operation, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;
private:
    uint32_t calculateSelfOplogIdFromNodeOplogId(const OperationBody &body, OperatorDependencies dependencies);
};