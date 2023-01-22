#pragma once

#include "operators/Operator.h"
#include "./messages/response/ErrorCode.h"

#include <string>
#include <memory>

class GetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x02;

    Response operate(const OperationBody& operation, std::shared_ptr<Map> map) override {
        std::optional<MapEntry> result = map->get(operation.args->at(0));

        if(result.has_value() && result.value().hasValue())
            result.value().value.increaseRefCount();

        return result.has_value() && result.value().hasValue() ?
               Response::success(result.value().value) :
               Response::error(ErrorCode::UNKNOWN_KEY); //No successful
    }

    constexpr OperatorType type() override {
        return READ;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};