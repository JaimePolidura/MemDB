#pragma once

#include "shared.h"
#include "replication/NodeState.h"
#include "server/Connection.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"

struct Node {
public:
    std::shared_ptr<io_context> ioContext = std::make_shared<io_context>();

    connection_t connection;
    std::string address;
    NodeState state;
    int nodeId;

    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;

    Response sendRequest(const Request& request, const bool includesNodeId = false) {
        this->openConnectionIfClosed();

        std::vector<uint8_t> serializedRequest = this->requestSerializer.serialize(request, includesNodeId);
        this->connection->writeSync(serializedRequest);

        std::vector<uint8_t> serializedResponse = this->connection->readSync();
        Response deserializedResponse = this->responseDeserializer.deserialize(serializedResponse);

        return deserializedResponse;
    }

    void closeConnection() {
        this->connection->close();
    }

    void openConnection() {
        if(!NodeStates::canAcceptRequest(this->state) ||
           (this->connection.get() != nullptr && this->connection->isOpen()))
            return;

        auto splitedAddress = StringUtils::split(this->address, ':');
        auto ip = splitedAddress[0];
        auto port = splitedAddress[1];

        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), std::atoi(port.data()));
        boost::asio::ip::tcp::socket socket(* this->ioContext);
    }

private:
    void openConnectionIfClosed() {
        if(this->connection.get() == nullptr || !this->connection->isOpen()){
            this->openConnection();
        }
    }

public:
    static bool canSendRequestUnicast(const Node& node) {
        return node.state == NodeState::RUNNING;
    }

    static std::string toJson(const Node& node) {
        return "{\"nodeId\": \""+std::to_string(node.nodeId)+"\", \"address\": \""+node.address+"\", \"state\": \""+NodeStates::parseNodeStateToString(node.state)+"\"}";
    }

    static Node fromJson(const nlohmann::json& json) {
        return Node{
                .address = json["address"].get<std::string>(),
                .state = NodeStates::parseNodeStateFromString(json["state"].get<std::string>()),
                .nodeId = json["nodeId"].get<int>(),
        };
    }
};