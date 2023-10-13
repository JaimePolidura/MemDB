#include "NextFragmentOperator.h"

Response NextFragmentOperator::operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies& dependencies) {
    uint64_t multiId = options.requestNumber;

    iterator_t<std::vector<uint8_t>> senderIterator = dependencies.multipleResponses->getSenderIteratorByMultiId(multiId);
    dependencies.multipleResponses->markFragmentSend(multiId);

    if(senderIterator->hasNext()) {
        return Response::success(SimpleString<memDbDataLength_t>::fromVector(senderIterator->next()));
    } else {
        return Response::success();
    }
}

OperatorDescriptor NextFragmentOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::NEXT_FRAGMENT,
        .name = "NEXT_FRAGMENT",
        .authorizedToExecute = { AuthenticationType::NODE },
    };
}