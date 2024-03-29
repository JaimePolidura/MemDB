#include "GetNodeDataOperator.h"

Response GetNodeDataOperator::operate(const OperationBody&operation, const OperationOptions operationOptions, OperatorDependencies&dependencies) {
    memdbNodeId_t nodeId = operation.getArg(0).to<memdbNodeId_t>();
    std::optional<node_t> nodeOptional = dependencies.cluster->clusterNodes->getByNodeId(nodeId);
    bool isSuccessful = nodeOptional.has_value();

    return ResponseBuilder::builder()
        .valueIfSuccessful(isSuccessful, this->nodeToSimpleString(nodeOptional.value()))
        ->isSuccessful(isSuccessful, ErrorCode::NODE_NOT_FOUND)
        ->build();
}

SimpleString<memDbDataLength_t> GetNodeDataOperator::nodeToSimpleString(node_t node) {
    return SimpleString<memDbDataLength_t>::fromSimpleStrings({
        SimpleString<memDbDataLength_t>::fromNumber(node->nodeId),
        SimpleString<memDbDataLength_t>::fromNumber(static_cast<uint32_t>(node->address.size())),
        SimpleString<memDbDataLength_t>::fromString(node->address),
    });
}

OperatorDescriptor GetNodeDataOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number =  OperatorNumbers::GET_NODE_DATA,
        .name = "GET_NODE_DATA",
        .authorizedToExecute = { AuthenticationType::NODE, AuthenticationType::USER },
        .properties = {}
    };
}
