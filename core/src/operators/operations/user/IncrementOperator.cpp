#include "IncrementOperator.h"

Response IncrementOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies&dependencies) {

}

OperatorDescriptor IncrementOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::DB_STORE_WRITE,
        .number = OperatorNumbers::INC,
        .name = "INCREMENT",
        .authorizedToExecute = { AuthenticationType::NODE, AuthenticationType::USER }
    };
}

