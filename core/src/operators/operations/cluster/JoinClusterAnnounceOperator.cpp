#include "JoinClusterAnnounceOperator.h"

Response JoinClusterAnnounceOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies&dependencies) {
    std::string address = operation.getArg(0).toString();
    memdbNodeId_t newNodeId = operation.nodeId;

    dependencies.logger->info("Received JOIN_CLUSTER_ANNOUNCE from nodeId {0} with address {1}", newNodeId, address);

    dependencies.cluster->newNodeInClusterHandler(std::make_shared<Node>(newNodeId, address,
        dependencies.configuration->get<uint64_t>(ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS)));

    return Response::success();
}

OperatorDescriptor JoinClusterAnnounceOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::JOIN_CLUSTER_ANNOUNCE,
        .name = "JOIN_CLUSTER_ANNOUNCE",
        .authorizedToExecute = { AuthenticationType::NODE}
    };
}
