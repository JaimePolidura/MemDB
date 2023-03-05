#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperator.h"

class GetOperator : public Operator, public DbOperator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x02;

    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, memDbDataStore_t map) override {
        std::optional<MapEntry<defaultMemDbLength_t>> result = map->get(operation.args->at(0));

        return result.has_value() ?
               Response::success(result.value().value) :
               Response::error(ErrorCode::UNKNOWN_KEY); //No successful
    }

    AuthenticationType authorizedToExecute() override {
        return AuthenticationType::USER;
    }

    constexpr OperatorType type() override {
        return OperatorType::READ;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};