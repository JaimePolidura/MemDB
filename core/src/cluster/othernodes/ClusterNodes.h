#pragma once

#include "shared.h"

#include "cluster/Node.h"
#include "utils/strings/StringUtils.h"
#include "utils/threads/pool/ThreadPool.h"
#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "logging/Logger.h"
#include "cluster/othernodes/NodeGroup.h"
#include "cluster/othernodes/NodeGroupOptions.h"

class ClusterNodes {
private:
    std::map<memdbNodeId_t, node_t> nodesById;
    std::vector<NodeGroup> groups;
    
    configuration_t configuration;
    ThreadPool requestPool;
    logger_t logger;

public:
    ClusterNodes(configuration_t configuration, logger_t logger):
        logger(logger),
        nodesById(std::map<memdbNodeId_t, node_t>{}),
        configuration(configuration),
        requestPool(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_SERVER_THREADS)) {}
        
    ClusterNodes(): nodesById() {}

    void setOtherNodes(const std::vector<node_t>& otherNodesToSet, const NodeGroupOptions options = {});

    std::vector<NodeGroup> getGroups();

    bool isEmtpy(const NodeGroupOptions options = {});

    void setNodeState(memdbNodeId_t nodeId, const NodeState newState);

    void addNode(node_t node, const NodeGroupOptions options = {});

    bool existsByNodeId(memdbNodeId_t nodeId);

    node_t getByNodeId(memdbNodeId_t nodeId);

    void deleteNodeById(const memdbNodeId_t nodeId);

    Response sendRequest(memdbNodeId_t nodeId, const Request& request);

    std::optional<Response> sendRequestToRandomNode(const Request& request, const NodeGroupOptions options = {});

    void broadcast(const OperationBody& operation, const NodeGroupOptions options = {});

    node_t getRandomNode(std::set<memdbNodeId_t> alreadyCheckedNodesId = {}, const NodeGroupOptions options = {});
private:
    Request prepareRequest(const OperationBody& operation);
};

using clusterNodes_t = std::shared_ptr<ClusterNodes>;