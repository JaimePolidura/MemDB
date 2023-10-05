#include "operators/operations/cluster/HealthCheckOperator.h"

Response HealthCheckOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    return Response::success();
}

std::vector<AuthenticationType> HealthCheckOperator::authorizedToExecute() {
    return { AuthenticationType::MAINTENANCE };
}

constexpr OperatorType HealthCheckOperator::type() {
    return OperatorType::NODE_MAINTENANCE;
}

constexpr uint8_t HealthCheckOperator::operatorNumber() {
    return OPERATOR_NUMBER;
}

std::string HealthCheckOperator::name() {
    return "HEALTH_CHECK";
}
