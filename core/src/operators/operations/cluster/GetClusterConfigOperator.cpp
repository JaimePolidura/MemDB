#include "GetClusterConfigOperator.h"

Response GetClusterConfigOperator::operate(const OperationBody& operation, const OperationOptions operationOptions, OperatorDependencies& dependencies) {
    bool usingPartitions = dependencies.configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS);
    int nNodesCluster = dependencies.cluster->clusterNodes->getSize();

    dependencies.logger->info("Received GET_CLUSTER_CONFIG from node {0}", operation.nodeId);

    return ResponseBuilder::builder()
        .success()
        ->values({
            SimpleString<memDbDataLength_t>::fromNumber(dependencies.cluster->getNodesPerPartition()),
            SimpleString<memDbDataLength_t>::fromNumber(dependencies.cluster->getMaxPartitionSize()),
            SimpleString<memDbDataLength_t>::fromNumber(nNodesCluster + 1), //clusterNodes->getSize() doest include our self
            SimpleString<memDbDataLength_t>::fromSimpleStrings(
                nodesToSimpleStrings(dependencies, dependencies.cluster->clusterNodes->getAllNodes())
            ),
            SimpleString<memDbDataLength_t>::fromSimpleStrings(
                ringEntriesToSimpleStrings(usingPartitions, dependencies.cluster->partitions->getAll())
            ),
        })
        ->build();
}

std::vector<SimpleString<memDbDataLength_t>> GetClusterConfigOperator::ringEntriesToSimpleStrings(bool usingPartitions, const std::vector<RingEntry>& ringEntries) {
    if(!usingPartitions){
        return {};
    }

    std::vector<SimpleString<memDbDataLength_t>> nodesToReturn{};

    std::for_each(ringEntries.begin(), ringEntries.end(), [&nodesToReturn](const RingEntry& ringEntry){
        nodesToReturn.push_back(SimpleString<memDbDataLength_t>::fromSimpleStrings({
            SimpleString<memDbDataLength_t>::fromNumber(ringEntry.nodeId),
            SimpleString<memDbDataLength_t>::fromNumber(ringEntry.ringPosition),
        }));
    });

    return nodesToReturn;
}

std::vector<SimpleString<memDbDataLength_t>> GetClusterConfigOperator::nodesToSimpleStrings(OperatorDependencies& dependencies, const std::vector<node_t>& nodes) {
    std::vector<SimpleString<memDbDataLength_t>> nodesToReturn{};

    std::for_each(nodes.begin(), nodes.end(), [&nodesToReturn](const node_t& node){
        nodesToReturn.push_back(SimpleString<memDbDataLength_t>::fromSimpleStrings({
            SimpleString<memDbDataLength_t>::fromNumber(node->nodeId),
            SimpleString<memDbDataLength_t>::fromNumber(static_cast<uint32_t>(node->address.size())),
            SimpleString<memDbDataLength_t>::fromString(node->address),
        }));
    });

    //We include our self
    std::string selfAddress = dependencies.configuration->get(ConfigurationKeys::ADDRESS);
    nodesToReturn.push_back(SimpleString<memDbDataLength_t>::fromNumber(dependencies.cluster->getNodeId()));
    nodesToReturn.push_back(SimpleString<memDbDataLength_t>::fromNumber(static_cast<uint32_t>(selfAddress.size())));
    nodesToReturn.push_back(SimpleString<memDbDataLength_t>::fromString(selfAddress));

    return nodesToReturn;
}

OperatorDescriptor GetClusterConfigOperator::desc() {
    return OperatorDescriptor{
        .type = OperatorType::NODE_MAINTENANCE,
        .number = OperatorNumbers::GET_CLUSTER_CONFIG,
        .name = "GET_CLUSTER_CONFIG",
        .authorizedToExecute = { AuthenticationType::NODE, AuthenticationType::USER }
    };
}