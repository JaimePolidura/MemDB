#include "operators/operations/user/GetOperator.h"

Response GetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    std::optional<MapEntry<memDbDataLength_t>> result = dependencies.dbStore->get(operation.args->at(0));

    return result.has_value() ?
        Response::success(result.value().value) :
        Response::error(ErrorCode::UNKNOWN_KEY); //No successful
}

constexpr OperatorDescriptor DeleteOperator::desc() {
    return OperatorDescriptor{
            .type = OperatorType::DB_STORE_READ,
            .number = OPERATOR_NUMBER,
            .name = "GET",
            .authorizedToExecute = { AuthenticationType::API },
            .isMulti = false,
    };
}