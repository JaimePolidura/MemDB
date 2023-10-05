#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "persistence/OperationLogDeserializer.h"
#include "persistence/OperationLogInvalidator.h"
#include "persistence/OperationLogUtils.h"

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

    OperatorType type() override;

    std::vector<AuthenticationType> authorizedToExecute() override;

    std::string name() override;

    constexpr uint8_t operatorNumber() override;

private:
    void clearOperationLog(OperatorDependencies dependencies, uint32_t operationLogId);
};