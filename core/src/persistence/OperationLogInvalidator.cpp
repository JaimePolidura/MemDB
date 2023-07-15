#include "persistence/OperationLogInvalidator.h"

std::vector<OperationBody> OperationLogInvalidator::getInvalidationOperations(const std::vector<OperationBody> &toInvalidate) {
    std::vector<SimpleString<memDbDataLength_t>> keys = OperationLogUtils::getUniqueKeys(toInvalidate);
    std::vector<OperationBody> deleteOperations(keys.size());

    std::transform(keys.begin(), keys.end(), deleteOperations.begin(),
                   [this](SimpleString<memDbDataLength_t> key ) -> OperationBody{ return this->createDeleteKeyOperation(key);});

    return deleteOperations;
}

OperationBody OperationLogInvalidator::createDeleteKeyOperation(const SimpleString<memDbDataLength_t>& key) {
    OperationBody operationBody{};
    operationBody.operatorNumber = 0x03; //DeleteOperator
    args_t args = OperationBody::createOperationBodyArg();
    args->push_back(key);

    return operationBody;
}