#pragma once

#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ErrorCode.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/utils/OperationLogInvalidator.h"
#include "persistence/utils/OperationLogUtils.h"

class LeaveClusterAnnounceOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;

    OperatorDescriptor desc() override;
};
