#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "utils/Utils.h"
#include "persistence/serializers/OperationLogSerializer.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "config/keys/ConfigurationKeys.h"
#include "operators/OperatorDependencies.h"

/**
 * Args:
 *  - uint32 part1 Timestamp to sync
 *  - uint32 part2 Timestamp to sync
 *  - uint32 nodeOplogIdToSync
 */
class SyncOplogOperator : public Operator {
private:
    OperationLogSerializer operationLogSerializer;
    OperationLogCompacter operationLogCompacter{};

public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x05;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) override;

    constexpr OperatorDescriptor desc() override;
private:
    //Timestamp is 64 bits Actual memdb data size is 32 bits. Doest fit, we pass two args that consist of the two parts
    uint64_t parseUnsyncTimestampFromRequest(const OperationBody &operation) const;

    uint32_t calculateSelfOplogIdFromNodeOplogId(const OperationBody &body, OperatorDependencies dependencies);
};