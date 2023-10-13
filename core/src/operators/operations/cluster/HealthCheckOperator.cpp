#include "operators/operations/cluster/HealthCheckOperator.h"

Response HealthCheckOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    return Response::success();
}

OperatorDescriptor HealthCheckOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::NODE_MAINTENANCE,
            .number = OperatorNumbers::HEALTH_CHECK,
            .name = "HEALTH_CHECK",
            .authorizedToExecute = { AuthenticationType::MAINTENANCE },
    };
}