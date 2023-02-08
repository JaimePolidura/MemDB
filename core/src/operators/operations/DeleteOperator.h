#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

#include <string>
#include <memory>

class DeleteOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x03;

    Response operate(const OperationBody& operation, const OperationOptions& options, std::shared_ptr<Map> map) override {
        bool ignoreTimestmaps = !options.requestFromReplication;
        bool removed = map->remove(operation.args->at(0), ignoreTimestmaps, operation.timestamp, operation.nodeId);

        return removed ? Response::success() : Response::error(ErrorCode::UNKNOWN_KEY);
    }

    AuthenticationType authorizedToExecute() override {
        return AuthenticationType::USER;
    }

    constexpr OperatorType type() override {
        return OperatorType::WRITE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};