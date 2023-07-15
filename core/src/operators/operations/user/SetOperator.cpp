#include "operators/operations/user/SetOperator.h"

Response SetOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) {
    SimpleString key = operation.args->at(0);
    SimpleString value = operation.args->at(1);

    bool ignoreTimestmaps = !options.checkTimestamps;
    bool updated = dependencies.dbStore->put(key, value, ignoreTimestmaps, operation.timestamp, operation.nodeId);

    return updated ?
        Response::success() :
        Response::error(ErrorCode::ALREADY_REPLICATED);
}

std::vector<AuthenticationType> SetOperator::authorizedToExecute() {
    return { AuthenticationType::NODE, AuthenticationType::API };
}

std::vector<OperatorDependency> SetOperator::dependencies() {
    return { OperatorDependency::DB_STORE };
}

constexpr OperatorType SetOperator::type() {
    return OperatorType::DB_STORE_WRITE;
}

constexpr uint8_t SetOperator::operatorNumber() {
    return OPERATOR_NUMBER;
}

std::string SetOperator::name() {
    return "SET";
}