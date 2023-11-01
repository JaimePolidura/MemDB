#pragma once

#include "shared.h"
#include "cluster/othernodes/NodePartitionOptions.h"
#include "cluster/othernodes/MultipleResponses.h"
#include "cluster/othernodes/NodesInPartition.h"
#include "cluster/Node.h"

#include "utils/strings/StringUtils.h"
#include "utils/threads/pool/ThreadPool.h"

#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"

#include "logging/Logger.h"

class ClusterNodes {
private:
    std::map<memdbNodeId_t, node_t> nodesById{};
    std::vector<NodesInPartition> nodesInPartitions{};

    configuration_t configuration;
    ThreadPool requestPool;
    logger_t logger;

public:
    ClusterNodes(configuration_t configuration, logger_t logger):
        logger(logger),
        nodesById(std::map<memdbNodeId_t, node_t>{}),
        configuration(configuration),
        requestPool(configuration->get<int>(ConfigurationKeys::SERVER_THREADS)) {}
        
    ClusterNodes(): nodesById() {}

    void setNumberPartitions(uint32_t numberPartitions);

    void setOtherNodes(const std::vector<node_t>& otherNodesToSet, const NodePartitionOptions options);

    std::vector<NodesInPartition> getNodesInPartitions();

    bool isEmtpy(const NodePartitionOptions options);

    void setNodeState(memdbNodeId_t nodeId, const NodeState newState);

    void addNode(node_t node, const NodePartitionOptions options);

    bool existsByNodeId(memdbNodeId_t nodeId);

    node_t getByNodeId(memdbNodeId_t nodeId);

    void deleteNodeById(const memdbNodeId_t nodeId);

    std::result<Response> sendRequestToAnyNode(const Request& request, bool requiresSuccessfulResponse, const NodePartitionOptions options);

    std::result<Response> sendRequestToAnyNode(bool requiresSuccessfulResponse,
                                                 const NodePartitionOptions options,
                                                 std::function<Request(node_t nodeToSend)> requestCreator);

    Response sendRequest(memdbNodeId_t nodeId, const Request& request);

    std::result<Response> sendRequestToRandomNode(const Request& request, const NodePartitionOptions options);

    void broadcast(const NodePartitionOptions options, const OperationBody& operation);

    multipleResponses_t broadcastAndWait(const NodePartitionOptions options, const OperationBody& operation);

    void removeNodeFromPartition(memdbNodeId_t nodeId, const NodePartitionOptions options);
    
    std::optional<node_t> getRandomNode(const NodePartitionOptions options, std::set<memdbNodeId_t> alreadyCheckedNodesId = {});
private:
    Request prepareRequest(const OperationBody& operation);

    void forEachNodeInPartitionThatCanAcceptReq(int partitionId, std::function<void(node_t node)> consumer);
};

using clusterNodes_t = std::shared_ptr<ClusterNodes>;