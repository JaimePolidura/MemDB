#pragma once

#include "operators/Operator.h"
#include "operators/OperatorNumbers.h"
#include "messages/response/ErrorCode.h"
#include "messages/response/ResponseBuilder.h"
#include "persistence/serializers/OperationLogDeserializer.h"
#include "persistence/utils/OperationLogInvalidator.h"
#include "persistence/utils/OperationLogUtils.h"

class GetClusterConfigOperator : public Operator {
public:
    Response operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) override;
    
    OperatorDescriptor desc() override;

private:
    std::vector<node_t> getNodesToSendInResponse(OperatorDependencies& dependencies);
};