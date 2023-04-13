#pragma once

#include "shared.h"
#include "replication/NodeState.h"
#include "server/Connection.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "utils/net/DNSUtils.h"
#include "logging/Logger.h"

struct Node {
public:
    io_context ioContext;

    mutable connection_t connection;
    std::string address;
    NodeState state;
    memdbNodeId_t nodeId;

    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;

    Node() = default;

    Node(const Node& other) {
        this->connection = other.connection;
        this->address = other.address;
        this->state = other.state;
        this->nodeId = other.nodeId;
        this->responseDeserializer = other.responseDeserializer;
        this->requestSerializer = other.requestSerializer;
    }

    Response sendRequest(const Request& request, logger_t logger) {
        this->openConnectionIfClosed(logger);

        std::vector<uint8_t> serializedRequest = this->requestSerializer.serialize(request);
        std::cout << this->connection->isOpen() << std::endl;
        logger->debugInfo("8 {0} {1}", serializedRequest.size(), this->connection->isOpen());

        this->connection->writeAsync(serializedRequest, [logger](const boost::system::error_code& error, std::size_t bytes_transferred) -> void{
            logger->debugInfo("wtf");
            if(error){
                logger->debugInfo("Error");
            }

            logger->debugInfo("Info {0}", bytes_transferred);
        });

        std::this_thread::sleep_for(std::chrono::seconds(2));

        auto xd = this->connection->writeSync(serializedRequest);
        logger->debugInfo("9 {0}", xd);

        //TODO Handle missed write
        std::vector<uint8_t> serializedResponse = this->connection->readSync();
        logger->debugInfo("10 {0}", serializedResponse.size());
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

    bool openConnection(logger_t logger) {
        if(!NodeStates::canAcceptRequest(this->state) ||
           (this->connection.get() != nullptr && this->connection->isOpen())){
            logger->debugInfo("3.4 {0} {1}", NodeStates::parseNodeStateToString(this->state), this->nodeId);
            return false;
        }

        auto splitedAddress = StringUtils::split(this->address, ':');
        auto ip = splitedAddress[0];
        auto port = splitedAddress[1];

        boost::asio::ip::tcp::socket socket(this->ioContext);

        return Utils::retryUntil(10, std::chrono::seconds(2), [ip, port, &socket, this]() mutable -> void{
            if(DNSUtils::isName(ip))
                ip = DNSUtils::singleResolve(ip, port);

            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), std::atoi(port.data()));
            socket.connect(endpoint);

            this->connection = std::make_shared<Connection>(std::move(socket));
        });;
    }

private:
    void openConnectionIfClosed(logger_t logger) {
        if(this->isConnectionOpened()){
            this->openConnection(logger);
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