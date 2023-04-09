#pragma once

#include "shared.h"

#include "replication/Node.h"
#include "utils/strings/StringUtils.h"
#include "utils/threads/pool/FixedThreadPool.h"
#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"

class ClusterNodesConnections {
public:
    std::vector<Node> otherNodes;
private:
    configuration_t configuration;
    FixedThreadPool requestPool;

public:
    ClusterNodesConnections(configuration_t configuration): configuration(configuration), requestPool(configuration->get<int>(ConfigurationKeys::SERVER_MAX_THREADS)) {}

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
        node.openConnection();
    }

    bool existsByNodeId(int nodeId) {
        for(int i = 0; i < this->otherNodes.size(); i++){
            if(this->otherNodes.at(i).nodeId == nodeId){
                return true;
            }
        }

        return false;
    }

    void deleteNodeById(int nodeId) {
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

    void createConnections() {
        for (auto& node : this->otherNodes)
            node.openConnection();
    }

    auto sendRequestToRandomNode(const Request& request, const bool includeNodeId = false) -> Response {
        Node nodeToSendRequest = this->selectRandomNodeToSendRequest();

        return nodeToSendRequest.sendRequest(request, includeNodeId);
    }

    auto broadcast(const Request& request, const bool includeNodeId = false) -> void {
        for(Node& node : this->otherNodes){
            if(!NodeStates::canAcceptRequest(node.state))
                continue;

            this->requestPool.submit([node, request, includeNodeId]() mutable -> void {
                node.sendRequest(request, includeNodeId);
            });
        }
    }

private:
    Node selectRandomNodeToSendRequest() {
        std::set<int> alreadyCheckedNodesId{};
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