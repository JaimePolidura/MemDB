#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"

#include <string>

class SetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x01;

    Response operate(const OperationBody& operation, const OperationOptions& options, std::shared_ptr<Map<defaultMemDbSize_t>> map) override {
        SimpleString key = operation.args->at(0);
        SimpleString value = operation.args->at(1);

        bool ignoreTimestmaps = !options.requestFromReplication;
        bool updated = map->put(key, value, ignoreTimestmaps, operation.timestamp, operation.nodeId);

        return updated ?
            Response::success() :
            Response::error(ErrorCode::ALREADY_REPLICATED);
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