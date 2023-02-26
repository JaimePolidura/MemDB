#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/ControlOperator.h"

class HealthCheckOperator : public Operator, public ControlOperator {
    static constexpr const uint8_t OPERATOR_NUMBER = 0x04;

    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, operationLogBuffer_t operationLogBuffer) override {
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