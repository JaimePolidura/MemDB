#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperatorExecutor.h"

class GetOperator : public Operator, public DbOperatorExecutor {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x02;

    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, memDbDataStore_t map) override {
        std::optional<MapEntry<memDbDataLength_t>> result = map->get(operation.args->at(0));

        return result.has_value() ?
               Response::success(result.value().value) :
               Response::error(ErrorCode::UNKNOWN_KEY); //No successful
    }

    std::string name() override {
        return "GET";
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::API };
    }

    constexpr OperatorType type() override {
        return OperatorType::READ;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }
};