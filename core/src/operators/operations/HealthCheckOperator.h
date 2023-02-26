#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

#include <string>

class HealthCheckOperator : public Operator {
    static constexpr const uint8_t OPERATOR_NUMBER = 0x04;

    Response operateControl(const OperationBody& operation, const OperationOptions& operationOptions, std::shared_ptr<OperationLogBuffer> operationLogBuffer) override {
        return Response::success();
    }

    AuthenticationType authorizedToExecute() override {
        return AuthenticationType::CLUSTER;
    }

    constexpr OperatorType type() override {
        return OperatorType::CONTROL;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};