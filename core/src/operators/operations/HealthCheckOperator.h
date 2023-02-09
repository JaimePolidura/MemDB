#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

#include <string>

class HealthCheckOperator : public Operator {
    static constexpr const uint8_t OPERATOR_NUMBER = 0x04;

    Response operate(const OperationBody& operation, const OperationOptions& options, std::shared_ptr<Map> map) override {
        printf("Recieved health check\n");

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