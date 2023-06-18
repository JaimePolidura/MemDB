#pragma once

#include "shared.h"
#include "cluster/NodeState.h"
#include "server/Connection.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "utils/net/DNSUtils.h"
#include "logging/Logger.h"
#include "utils/Utils.h"

struct Node {
public:
    io_context ioContext;

    mutable connection_t connection;
    std::string address;
    NodeState state;
    memdbNodeId_t nodeId;

    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;

    Node(): connection(nullptr) {}

    Node(const Node& other) {
        this->connection = other.connection;
        this->address = other.address;
        this->state = other.state;
        this->nodeId = other.nodeId;
        this->responseDeserializer = other.responseDeserializer;
        this->requestSerializer = other.requestSerializer;
    }

    auto sendRequest(const Request& request, const bool waitForResponse) -> std::optional<Response> {
        this->openConnectionIfClosedOrThrow();
        
        std::vector<uint8_t> serializedRequest = this->requestSerializer.serialize(request);
        this->connection->writeSync(serializedRequest);

        if(!waitForResponse)
            return std::nullopt;

        std::vector<uint8_t> serializedResponse = this->connection->readSync();
        Response deserializedResponse = this->responseDeserializer.deserialize(serializedResponse);

        return deserializedResponse;
    }

    void closeConnection() {
        if(this->isConnectionOpened()){
            this->connection->close();
        }
    }

    bool isConnectionOpened() const {
        return this->connection.get() != nullptr && this->connection->isOpen();
    }

    bool openConnection() {
        if(!NodeStates::canAcceptRequest(this->state) || this->isConnectionOpened()){
            return false;
        }

        auto splitedAddress = StringUtils::split(this->address, ':');
        auto ip = splitedAddress[0];
        auto port = splitedAddress[1];

        boost::asio::ip::tcp::socket socket(this->ioContext);

        bool success = Utils::tryOnce([ip, port, &socket]() mutable -> void{
            if(DNSUtils::isName(ip))
                ip = DNSUtils::singleResolve(ip, port);

            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), std::atoi(port.data()));
            socket.connect(endpoint);
        });

        if(!success)
            return false;

        this->connection = std::make_shared<Connection>(std::move(socket));

        return true;
    }

private:
    void openConnectionIfClosedOrThrow() {
        if(!this->isConnectionOpened()){
            bool success = this->openConnection();

            if(!success){
                throw std::runtime_error("Cannot open connection");
            }
        }
    }

public:
    static bool canSendRequestUnicast(NodeState state) {
        return state == NodeState::RUNNING;
    }

    static std::string toJson(std::shared_ptr<Node> node) {
        return "{\"nodeId\": \""+std::to_string(node->nodeId)+"\", \"address\": \""+node->address+"\", \"state\": \""+NodeStates::parseNodeStateToString(node->state)+"\"}";
    }

    static std::shared_ptr<Node> fromJson(const nlohmann::json& json) {
        std::shared_ptr<Node> node = std::make_shared<Node>();
        node->address = json["address"].get<std::string>();
        node->state = NodeStates::parseNodeStateFromString(json["state"].get<std::string>());
        node->nodeId = (memdbNodeId_t) std::stoi(json["nodeId"].get<std::string>());

        return node;
    }
};

using node_t = std::shared_ptr<Node>;