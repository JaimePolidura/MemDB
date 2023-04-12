#pragma once

#include "shared.h"

#include "replication/Node.h"
#include "utils/strings/StringUtils.h"
#include "utils/threads/pool/FixedThreadPool.h"
#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "logging/Logger.h"

class ClusterNodesConnections {
public:
    std::vector<Node> otherNodes;
private:
    configuration_t configuration;
    FixedThreadPool requestPool;
    logger_t logger;

public:
    ClusterNodesConnections(configuration_t configuration, logger_t logger):
        logger(logger),
        configuration(configuration),
        requestPool(configuration->get<int>(ConfigurationKeys::MEMDB_CORE_SERVER_MAX_THREADS)) {}

    void setOtherNodes(const std::vector<Node>& otherNodesToSet) {
        this->otherNodes = otherNodesToSet;
    }

    void replaceNode(const Node& node) {
        for(int i = 0; i < this->otherNodes.size(); i++){
            if(this->otherNodes.at(i).nodeId == node.nodeId){
                this->otherNodes[i].state = node.state;
                return;
            }
        }
    }

    void addNode(Node& node) {
        this->otherNodes.push_back(node);
        node.openConnection(this->logger);
    }

    bool existsByNodeId(const std::string& nodeId) {
        for(int i = 0; i < this->otherNodes.size(); i++){
            if(this->otherNodes.at(i).nodeId == nodeId){
                return true;
            }
        }

        return false;
    }

    void deleteNodeById(const std::string& nodeId) {
        for(int i = 0; i < this->otherNodes.size(); i++){
            if(this->otherNodes.at(i).nodeId == nodeId){
                this->otherNodes.erase(this->otherNodes.begin() + i);
                break;
            }
        }
    }

    void deleteAllConnections() {
        for (Node& node : this->otherNodes)
            node.closeConnection();
    }

    int createConnections() {
        int numberConnectionsOpened = 0;
        for (auto& node : this->otherNodes) {
            bool opened = node.openConnection(this->logger);
            if(opened)
                numberConnectionsOpened++;
        }

        return numberConnectionsOpened;
    }

    auto sendRequestToRandomNode(const Request& request, const bool includeNodeId = false) -> Response {
        Node nodeToSendRequest = this->selectRandomNodeToSendRequest();

        return nodeToSendRequest.sendRequest(request, this->logger, includeNodeId);
    }

    auto broadcast(const Request& request, const bool includeNodeId = false) -> void {
        for(Node& node : this->otherNodes){
            if(!NodeStates::canAcceptRequest(node.state)) {
                this->logger->debugInfo("0 {0} {1}", NodeStates::parseNodeStateToString(node.state), node.nodeId);
                continue;
            }

            this->requestPool.submit([node, request, includeNodeId, this]() mutable -> void {
                node.sendRequest(request, this->logger, includeNodeId);
            });
        }
    }

private:
    Node selectRandomNodeToSendRequest() {
        std::set<std::string> alreadyCheckedNodesId{};
        std::srand(std::time(nullptr));

        while(alreadyCheckedNodesId.size() != otherNodes.size()) {
            Node randomNode = this->otherNodes[std::rand() % this->otherNodes.size()];

            if(Node::canSendRequestUnicast(randomNode))
                return randomNode;

            alreadyCheckedNodesId.insert(randomNode.nodeId);
        }

        throw std::runtime_error("No nodes available to sync data, try later");
    }
};

using clusterNodesConnections_t = std::shared_ptr<ClusterNodesConnections>;