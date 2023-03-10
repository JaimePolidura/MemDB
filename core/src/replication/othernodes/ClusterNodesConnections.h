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
    std::map<int, boost::asio::ip::tcp::socket> sockets; //NodeId -> tcp socket
    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;
    boost::asio::io_context ioContext;
    FixedThreadPool requestPool;

public:
    ClusterNodesConnections(configuration_t configuration, const std::vector<Node>& otherNodes): configuration(configuration),
        requestPool(configuration->get<int>(ConfigurationKeys::SERVER_MAX_THREADS)), otherNodes(otherNodes) {
    }

    void replaceNode(const Node& node) {
        for(int i = 0; i < this->otherNodes.size(); i++){
            if(this->otherNodes.at(i).nodeId == node.nodeId){
                this->otherNodes[i] = node;
                return;
            }
        }
    }

    void addNode(const Node& node) {
        this->otherNodes.push_back(node);
        this->createSocket(node);
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
                this->sockets.erase(nodeId);
                break;
            }
        }
    }

    void deleteAllConnections() {
        for (const auto& node : this->otherNodes) {
            if(this->sockets.contains(node.nodeId)) {
                this->sockets.at(node.nodeId).close();
                this->sockets.erase(node.nodeId);
            }
        }
    }

    void createConnections() {
        for (const auto& node : this->otherNodes)
            this->createSocket(node);
    }

    auto sendRequestToRandomNode(const Request& request, const bool includeNodeId = false) -> Response {
        auto nodeToSendRequest = this->selectRandomNodeToSendRequest();

        return this->sendRequestToNode(nodeToSendRequest, request);
    }

    auto broadcast(const Request& request, const bool includeNodeId = false) -> void {
        for(const auto& node : this->otherNodes){
            if(!NodeStates::canAcceptRequest(node.state))
                continue;

            this->requestPool.submit([node, request, includeNodeId, this](){
                this->sendRequestToNode(node, request, includeNodeId);
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

    Response sendRequestToNode(const Node& node, const Request& request, const bool includeNodeId = false) {
        const_cast<Request&>(request).authentication.authKey = this->configuration->get(ConfigurationKeys::AUTH_CLUSTER_KEY);

        auto socket = std::move(this->sockets.at(node.nodeId));
        auto requestBytes = this->requestSerializer.serialize(request, includeNodeId);

        socket.write_some(boost::asio::buffer(requestBytes));

        std::vector<uint8_t> responseHeaderBuffer(21);
        socket.read_some(boost::asio::buffer(responseHeaderBuffer));
        auto responseBodyLenght = Utils::parseFromBuffer<uint32_t>(responseHeaderBuffer, 17);
        std::vector<uint8_t> responseBodyBuffer(responseBodyLenght);
        responseHeaderBuffer.insert(responseHeaderBuffer.end(), responseBodyBuffer.begin(), responseBodyBuffer.end());

        return responseDeserializer.deserialize(responseHeaderBuffer);
    }

    void createSocket(const Node& node) {
        if(!NodeStates::canAcceptRequest(node.state) || this->sockets.count(node.nodeId) == 1 || this->sockets.at(node.nodeId).is_open())
            return;

        auto splitedAddress = StringUtils::split(node.address, ':');
        auto ip = splitedAddress[0];
        auto port = splitedAddress[1];

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), std::atoi(port.data()));
        boost::asio::ip::tcp::socket socket(this->ioContext);
        socket.connect(endpoint);

        this->sockets.insert({node.nodeId, std::move(socket)});
    }
};

using clusterNodesConnections_t = std::shared_ptr<ClusterNodesConnections>;