#pragma once

#include "database/operators/Operator.h"
#include "./messages/response/ErrorCode.h"

#include <string>
#include <memory>

class GetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x02;

    std::shared_ptr<Response> operate(std::shared_ptr<OperationBody> operation, std::shared_ptr<Map> map) override {
        const std::string& key = std::string((char *) operation->args[0].arg, operation->args[0].lengthArg);

        std::optional<MapEntry> result = map->get(key);

        return result.has_value() ?
               Response::success(result.value().valueSize, result.value().value) :
               Response::error(ErrorCode::UNKNOWN_KEY); //No successful
    }

    constexpr OperatorType type() override {
        return READ;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};