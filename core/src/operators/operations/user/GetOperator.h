#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "operators/DbOperatorExecutor.h"

class GetOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x02;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override {
        std::optional<MapEntry<memDbDataLength_t>> result = dependencies.dbStore->get(operation.args->at(0));

        return result.has_value() ?
               Response::success(result.value().value) :
               Response::error(ErrorCode::UNKNOWN_KEY); //No successful
    }

    std::vector<OperatorDependency> dependencies() override {
        return { OperatorDependency::DB_STORE };
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::API };
    }

    constexpr OperatorType type() override {
        return OperatorType::DB_STORE_READ;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

    std::string name() override {
        return "GET";
    }
};