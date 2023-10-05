#include "operators/operations/user/DeleteOperator.h"

Response DeleteOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    bool ignoreTimestmaps = !options.checkTimestamps;
    bool removed = dependencies.dbStore->remove(operation.args->at(0), ignoreTimestmaps, operation.timestamp, operation.nodeId);

    return removed ? Response::success() : Response::error(ErrorCode::UNKNOWN_KEY);
}

std::vector<AuthenticationType> DeleteOperator::authorizedToExecute() {
    return { AuthenticationType::API, AuthenticationType::NODE };
}


OperatorType DeleteOperator::type() {
    return OperatorType::DB_STORE_WRITE;
}

uint8_t DeleteOperator::operatorNumber() {
    return OPERATOR_NUMBER;
}

std::string DeleteOperator::name() {
    return "SET";
}