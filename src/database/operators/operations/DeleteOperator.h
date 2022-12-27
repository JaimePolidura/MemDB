#pragma once

#include "database/operators/Operator.h"
#include "./messages/response/ErrorCode.h"

#include <string>
#include <memory>

class DeleteOperator : public Operator {
public:
    static const uint8_t OPERATOR_NUMBER = 0x03;

    std::shared_ptr<Response> operate(std::shared_ptr<OperationBody> operation, std::shared_ptr<Map> map) override {
        const std::string& key = std::string((char *) operation->args[0].arg, operation->args[0].lengthArg);

        map->remove(key);

        return Response::success();
    }

    OperatorType type() override {
        return WRITE;
    }

    uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};