#pragma once

#include "shared.h"

#include "replication/Node.h"
#include "utils/strings/StringUtils.h"
#include "utils/threads/pool/FixedThreadPool.h"
#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "logging/Logger.h"

class ClusterNodes {
public:
    std::vector<node_t> otherNodes;
private:
    configuration_t configuration;
    FixedThreadPool requestPool;
    logger_t logger;

public:
    ClusterNodes(configuration_t configuration, logger_t logger):
        logger(logger),
        configuration(configuration),
        requestPool(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_SERVER_MAX_THREADS)) {}

    void setOtherNodes(const std::vector<node_t>& otherNodesToSet) {
        this->otherNodes = otherNodesToSet;
    }

    void setNodeState(memdbNodeId_t nodeId, const NodeState newState) {
        for(int i = 0; i < this->otherNodes.size(); i++){
            if(this->otherNodes.at(i)->nodeId == nodeId){
                this->otherNodes[i]->state = newState;
                return;
            }
        }
    }

    bool isConnectionOpened(memdbNodeId_t nodeId) const {
        node_t node = this->getNodeById(nodeId);

        return node != nullptr && node->isConnectionOpened();
    }

    void setConnectionOfNode(memdbNodeId_t nodeId, connection_t connection) {
        node_t node = this->getNodeById(nodeId);

        if(node != nullptr){
            node->connection = connection;
        }
    }

    void addNode(node_t node) {
        this->otherNodes.push_back(node);
        node->openConnection(this->logger);
    }

    bool existsByNodeId(memdbNodeId_t nodeId) {
        return this->getNodeById(nodeId) != nullptr ;
    }

    void deleteNodeById(const memdbNodeId_t nodeId) {
        for(int i = 0; i < this->otherNodes.size(); i++){
            node_t actualNode = this->otherNodes.at(i);

            if(actualNode->nodeId == nodeId){
                actualNode->closeConnection();
                this->otherNodes.erase(this->otherNodes.begin() + i);
                break;
            }
        }
    }

    auto sendRequestToRandomNode(const Request& request) -> Response {
        node_t nodeToSendRequest = this->selectRandomNodeToSendRequest();

        return nodeToSendRequest->sendRequest(this->prepareRequest(request), this->logger);
    }

    auto broadcast(const Request& request) -> void {
        for(node_t node : this->otherNodes){
            if(!NodeStates::canAcceptRequest(node->state)) {
                continue;
            }

            node->sendRequest(this->prepareRequest(request), this->logger);

//            this->requestPool.submit([node, request, this]() mutable -> void {
//            });
        }
    }

private:
    node_t selectRandomNodeToSendRequest() {
        std::set<memdbNodeId_t> alreadyCheckedNodesId{};
        std::srand(std::time(nullptr));

        while(alreadyCheckedNodesId.size() != otherNodes.size()) {
            node_t randomNode = this->otherNodes[std::rand() % this->otherNodes.size()];

            if(Node::canSendRequestUnicast(randomNode->state))
                return randomNode;

            alreadyCheckedNodesId.insert(randomNode->nodeId);
        }

        throw std::runtime_error("No nodes available to sync data, try later");
    }

    node_t getNodeById(memdbNodeId_t nodeId) const {
        for(int i = 0; i < this->otherNodes.size(); i++) {
            const node_t actualNode = * (this->otherNodes.data() + i);

            if(actualNode->nodeId == nodeId)
                return actualNode;
        }

        return nullptr;
    }

    Request prepareRequest(const Request& request) {
        const_cast<Request&>(request).authentication.authKey = this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY);
        const_cast<Request&>(request).operation.nodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        const_cast<Request&>(request).authentication.flag1 = true;

        return request;
    }
};

using clusterNodes_t = std::shared_ptr<ClusterNodes>;