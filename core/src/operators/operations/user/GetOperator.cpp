#include "operators/operations/user/GetOperator.h"

Response GetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) {
    std::optional<MapEntry<memDbDataLength_t>> result = dependencies.dbStore->get(operation.args->at(0));

    return result.has_value() ?
        Response::success(result.value().value) :
        Response::error(ErrorCode::UNKNOWN_KEY); //No successful
}

std::vector<OperatorDependency> GetOperator::dependencies() {
    return { OperatorDependency::DB_STORE };
}

std::vector<AuthenticationType> GetOperator::authorizedToExecute() {
    return { AuthenticationType::API };
}

constexpr OperatorType GetOperator::type() {
    return OperatorType::DB_STORE_READ;
}

constexpr uint8_t GetOperator::operatorNumber() {
    return OPERATOR_NUMBER;
}

std::string GetOperator::name() {
    return "GET";
}