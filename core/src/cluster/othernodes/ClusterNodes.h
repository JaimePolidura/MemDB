#pragma once

#include "shared.h"

#include "cluster/Node.h"
#include "utils/strings/StringUtils.h"
#include "utils/threads/pool/FixedThreadPool.h"
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
    FixedThreadPool requestPool;
    logger_t logger;

public:
    ClusterNodes(configuration_t configuration, logger_t logger):
        logger(logger),
        configuration(configuration),
        requestPool(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_SERVER_MAX_THREADS)) {}

    void setOtherNodes(const std::vector<node_t>& otherNodesToSet, const NodeGroupOptions options = {}) {
        for (const node_t& node: otherNodesToSet) {
            if(this->nodesById.contains(node->nodeId)) {
                this->nodesById[node->nodeId]->closeConnection();
                this->nodesById.erase(node->nodeId);
            }

            this->nodesById[node->nodeId] = node;
            this->groups[options.nodeGroupId].add(node->nodeId);
        }
    }

    bool isEmtpy(const NodeGroupOptions options = {}) {
        return options.nodeGroupId >= this->groups.size() || this->groups[options.nodeGroupId].size() == 0;
    }

    void setNodeState(memdbNodeId_t nodeId, const NodeState newState) {
        this->nodesById[nodeId]->state = newState;
    }

    void addNode(node_t node, const NodeGroupOptions options = {}) {
        this->groups[options.nodeGroupId].add(node->nodeId);
        this->nodesById[node->nodeId] = node;
    }

    bool existsByNodeId(memdbNodeId_t nodeId) {
        return this->nodesById.count(nodeId) != 0;
    }

    void deleteNodeById(const memdbNodeId_t nodeId) {
        this->nodesById.erase(nodeId);

        for(NodeGroup& group : this->groups){
            group.remove(nodeId);
        }
    }

    auto sendRequest(memdbNodeId_t nodeId, const Request& request) -> Response {
        node_t node = this->nodesById.at(nodeId);

        return Utils::retryUntilSuccessAndGet<Response, std::milli>(std::chrono::milliseconds(100), [node, &request]() -> Response {
            return node->sendRequest(request, true).value();
        });
    }

    auto sendRequestToRandomNode(const Request& request, const NodeGroupOptions options = {}) -> std::optional<Response> {
        std::set<memdbNodeId_t> alreadyCheckedNodesId = {};

        return Utils::retryUntilAndGet<Response, std::milli>(10, std::chrono::milliseconds(100), [this, &request, &alreadyCheckedNodesId, options]() -> Response {
            node_t nodeToSendRequest = this->getRandomNode(alreadyCheckedNodesId, options);

            return nodeToSendRequest->sendRequest(this->prepareRequest(request.operation), true).value();
        });
    }

    auto broadcast(const OperationBody& operation, const NodeGroupOptions options = {}) -> void {
        std::set<memdbNodeId_t> allNodesIdInGroup = this->groups[options.nodeGroupId].getAll();

        for(memdbNodeId_t nodeId : allNodesIdInGroup){
            node_t node = this->nodesById.at(nodeId);

            if(!NodeStates::canAcceptRequest(node->state)) {
                continue;
            }

            this->requestPool.submit([node, operation, this]() mutable -> void {
                Utils::retryUntil(10, std::chrono::milliseconds(100), [this, &node, &operation]() -> void{
                    node->sendRequest(this->prepareRequest(operation), false);
                });
            });
        }
    }

    node_t getRandomNode(std::set<memdbNodeId_t> alreadyCheckedNodesId = {}, const NodeGroupOptions options = {}) {
        std::srand(std::time(nullptr));
        NodeGroup group = this->groups[options.nodeGroupId];
        std::set<memdbNodeId_t> alreadyChecked{};
        std::set<memdbNodeId_t> nodesInGroup = group.getAll();

        while(alreadyCheckedNodesId.size() != group.size()) {
            memdbNodeId_t offset = std::rand() % group.size();

            if(alreadyChecked.contains(offset))
                continue;

            auto ptr = std::begin(nodesInGroup);
            std::advance(ptr, offset);
            node_t randomNode = this->nodesById.at(* ptr);

            if(Node::canSendRequestUnicast(randomNode->state))
                return randomNode;

            alreadyCheckedNodesId.insert(randomNode->nodeId);
        }

        throw std::runtime_error("No node available for selecting");
    }

private:
    Request prepareRequest(const OperationBody& operation) {
        Request request{};

        AuthenticationBody authenticationBody{};
        authenticationBody.authKey = this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY);
        authenticationBody.flag1 = true;

        const_cast<OperationBody&>(operation).nodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);

        request.operation = operation;
        request.authentication = authenticationBody;

        return request;
    }
};

using clusterNodes_t = std::shared_ptr<ClusterNodes>;