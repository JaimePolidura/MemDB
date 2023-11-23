#pragma once

#include "shared.h"
#include "cluster/othernodes/SendRequestOptions.h"
#include "cluster/othernodes/MultipleResponses.h"
#include "cluster/othernodes/SendRequestOptions.h"
#include "cluster/othernodes/NodesInPartition.h"
#include "cluster/Node.h"
#include "cluster/hints/HintsService.h"

#include "utils/strings/StringUtils.h"
#include "utils/threads/pool/ThreadPool.h"

#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"

#include "logging/Logger.h"

class ClusterNodes {
public:
    ClusterNodes(configuration_t configuration, logger_t logger):
        logger(logger),
        nodesById(std::map<memdbNodeId_t, node_t>{}),
        configuration(configuration),
        hintsService(std::make_shared<HintsService>(configuration)),
        requestPool(configuration->get<int>(ConfigurationKeys::SERVER_THREADS)) {}

    std::result<Response> sendRequestToAnyNode(const Request& request, bool requiresSuccessfulResponse, SendRequestOptions options);

    std::result<Response> sendRequestToAnyNode(bool requiresSuccessfulResponse,
        SendRequestOptions options,
        std::function<Request(node_t nodeToSend)> requestCreator);

    Response sendRequest(memdbNodeId_t nodeId, const Request& request);

    std::result<Response> sendRequestToRandomNode(const Request& request, SendRequestOptions options);

    void broadcast(const OperationBody& operation, SendRequestOptions options);

    void broadcastAll(const OperationBody& operation, SendRequestOptions options);

    multipleResponses_t broadcastAndWait(const OperationBody& operation, SendRequestOptions options);

    void setNumberPartitions(uint32_t numberPartitions);

    int getSize();

    void setOtherNodes(const std::vector<node_t>& otherNodesToSet, memdbPartitionId_t partitionId);

    std::vector<node_t> getAllNodes();

    std::vector<NodesInPartition> getNodesInPartitions();

    bool isEmtpy(memdbPartitionId_t partitionId);

    void addNode(node_t node, memdbPartitionId_t partitionId);

    bool existsByNodeId(memdbNodeId_t nodeId);

    std::optional<node_t> getByNodeId(memdbNodeId_t nodeId);

    void deleteNodeById(const memdbNodeId_t nodeId);

    void removeNodeFromPartition(memdbNodeId_t nodeId, memdbPartitionId_t partitionId);
    
    std::optional<node_t> getRandomNode(memdbPartitionId_t partitionId, std::set<memdbNodeId_t> alreadyCheckedNodesId = {});

    void sendHintedHandoff(memdbNodeId_t nodeId);

private:
    std::map<memdbNodeId_t, node_t> nodesById{};
    std::vector<NodesInPartition> nodesInPartitions{};

    configuration_t configuration;
    hintsService_t hintsService;
    ThreadPool requestPool;
    logger_t logger;

    Request prepareRequest(const OperationBody& operation);

    void forEachNodeInPartition(int partitionId, std::function<void(node_t node)> consumer);
};

using clusterNodes_t = std::shared_ptr<ClusterNodes>;