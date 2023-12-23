#include "GetClusterConfigOperator.h"

Response GetClusterConfigOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    bool usingPartitions = dependencies.configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS);

    dependencies.logger->info("Received GET_CLUSTER_CONFIG from node {0}", operation.nodeId);

    GetClusterConfigResponse response = {
        .nodesPerPartition = dependencies.cluster->getNodesPerPartition(),
        .maxPartitionSize = dependencies.cluster->getNodesPerPartition(),
        .nodes = getNodesToSendInResponse(dependencies),
        .ringEntries = usingPartitions ? dependencies.cluster->partitions->getAll() : std::vector<RingEntry>{}
    };

    return ResponseBuilder::builder()
        .success()
        ->value(SimpleString<memDbDataLength_t>::fromVector(GetClusterConfigResponse{
            .nodesPerPartition = dependencies.cluster->getNodesPerPartition(),
            .maxPartitionSize = dependencies.cluster->getMaxPartitionSize(),
            .nodes = getNodesToSendInResponse(dependencies),
            .ringEntries = usingPartitions ? dependencies.cluster->partitions->getAll() : std::vector<RingEntry>{}
        }.serialize()))
        ->build();
}

std::vector<node_t> GetClusterConfigOperator::getNodesToSendInResponse(OperatorDependencies& dependencies) {
    std::vector<node_t> nodesToSend = dependencies.cluster->clusterNodes->getAllNodes();
    nodesToSend.push_back(std::make_shared<Node>(
        dependencies.configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID),
        dependencies.configuration->get(ConfigurationKeys::ADDRESS) + ":" + dependencies.configuration->get(ConfigurationKeys::SERVER_PORT),
        0));

    return nodesToSend;
}

OperatorDescriptor GetClusterConfigOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::GET_CLUSTER_CONFIG,
        .name = "GET_CLUSTER_CONFIG",
        .authorizedToExecute = { AuthenticationType::NODE, AuthenticationType::USER },
        .properties = {}
    };
}