#pragma once

#include <map>
#include <boost/asio.hpp>
#include <vector>
#include <set>

#include "replication/Node.h"
#include "utils/strings/StringUtils.h"
#include "messages/request/Request.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"

class ClusterNodesConnections {
private:
    std::map<int, boost::asio::ip::tcp::socket> sockets; //NodeId -> tcp socket
    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;
    boost::asio::io_context ioContext;
    std::vector<Node> otherNodes;
public:
    ClusterNodesConnections() = default;

    void createSocketsToNodes(const std::vector<Node>& nodes) {
        this->otherNodes = nodes;

        for (const auto& node : nodes) {
            this->createSocket(node);
        }
    }

    auto sendRequest(const Request& request, const bool includeNodeId = false) -> Response {
        auto nodeToSendRequest = this->selectRandomNodeToSendRequest();
        auto socket = std::move(this->sockets.at(nodeToSendRequest.nodeId));
        auto requestBytes = this->requestSerializer.serialize(request, includeNodeId);

        socket.write_some(boost::asio::buffer(requestBytes));

        std::vector<uint8_t> responseHeaderBuffer(21);
        socket.read_some(boost::asio::buffer(responseHeaderBuffer));
        auto responseBodyLenght = Utils::parseFromBuffer<uint32_t>(responseHeaderBuffer, 17);
        std::vector<uint8_t> responseBodyBuffer(responseBodyLenght);
        responseHeaderBuffer.insert(responseHeaderBuffer.end(), responseBodyBuffer.begin(), responseBodyBuffer.end());
        
        return responseDeserializer.deserialize(responseHeaderBuffer);
    }

private:
    Node selectRandomNodeToSendRequest() {
        std::set<int> alreadyCheckedNodesId{};
        std::srand(std::time(nullptr));

        while(alreadyCheckedNodesId.size() != otherNodes.size()) {
            Node randomNode = this->otherNodes[std::rand() % this->otherNodes.size()];

            if(Nodes::canSendRequestUnicast(randomNode))
                return randomNode;

            alreadyCheckedNodesId.insert(randomNode.nodeId);
        }

        throw std::runtime_error("No nodes available to sync data, try later");
    }

    void createSocket(const Node& node) {
        if(this->sockets.count(node.nodeId) == 1 || this->sockets.at(node.nodeId).is_open())
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