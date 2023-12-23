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

    multipleResponses_t broadcastForEachAndWait(SendRequestOptions options, std::function<OperationBody(memdbNodeId_t)> requestCreator);

    void setNumberPartitions(uint32_t numberPartitions);

    int getSize();

    void addNodesInPartition(const std::vector<node_t>& otherNodesToSet, memdbPartitionId_t partitionId);

    std::vector<node_t> getAllNodes();

    std::vector<NodesInPartition> getNodesInPartitions();

    bool isEmtpy(memdbPartitionId_t partitionId);

    void addNode(node_t node);

    void addNodes(const std::vector<node_t>& nodes);

    void addNodeInPartition(node_t node, memdbPartitionId_t partitionId);

    bool existsByNodeId(memdbNodeId_t nodeId);

    std::optional<node_t> getByNodeId(memdbNodeId_t nodeId);

    void deleteNodeById(const memdbNodeId_t nodeId);

    void removeNodeFromPartition(memdbNodeId_t nodeId, memdbPartitionId_t partitionId);

    std::optional<node_t> getRandomNode(memdbPartitionId_t partitionId, std::set<memdbNodeId_t>& alreadyCheckedNodesId);

    void sendHintedHandoff(memdbNodeId_t nodeId);

private:
    //TODO Antipattern, expose globa thread pool instead
    friend class DoLeaveClusterOperator;
    friend class UpdateCounterOperator;

    std::map<memdbNodeId_t, node_t> allNodesById{}; //Includes all nodes even if the actual node doesnt share any partition

    std::vector<NodesInPartition> nodesInPartitions{};

    configuration_t configuration;
    hintsService_t hintsService;
    ThreadPool requestPool;
    logger_t logger;

    Request prepareRequest(const OperationBody& operation);

    void forEachNodeInPartition(int partitionId, std::function<void(node_t node)> consumer);
};

using clusterNodes_t = std::shared_ptr<ClusterNodes>;