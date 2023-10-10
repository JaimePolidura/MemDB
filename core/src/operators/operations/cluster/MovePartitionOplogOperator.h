#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/utils/OperationLogInvalidator.h"
#include "persistence/utils/OperationLogUtils.h"

/**
 * Flags:
 * - flag1 bool applyNewOplog
 * - flag2 bool clearOldOplog
 *
 * Args:
 *  - uint32_t newOplogId
 *  - uint32_t oldOplogId
 *  - std::vector<uint8_t> newOperationLog
 */
class MovePartitionOplogOperator : public Operator {
private:
    OperationLogDeserializer operationLogDeserializer{};
    OperationLogInvalidator operationLogInvalidator;

public:
    static constexpr int OPERATOR_NUMBER = 0x06;

    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;
    
    constexpr OperatorDescriptor desc() override;

private:
    void clearOperationLog(OperatorDependencies dependencies, uint32_t operationLogId);
};