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
    std::shared_ptr<io_context> ioContext = std::make_shared<io_context>();

    connection_t connection;
    std::string address;
    NodeState state;
    std::string nodeId;

    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;

    Response sendRequest(const Request& request, logger_t logger, const bool includesNodeId = false) {
        this->openConnectionIfClosed(logger);

        std::vector<uint8_t> serializedRequest = this->requestSerializer.serialize(request, includesNodeId);
        logger->debugInfo("8 {0}", serializedRequest.size());

        auto xd = this->connection->writeSync(serializedRequest);
        logger->debugInfo("9 {0}", xd);

        //TODO Handle missed write
        std::vector<uint8_t> serializedResponse = this->connection->readSync();
        logger->debugInfo("10 {0}", serializedResponse.size());
        Response deserializedResponse = this->responseDeserializer.deserialize(serializedResponse);

        return deserializedResponse;
    }

    void closeConnection() {
        if(this->connection.get() != nullptr && this->connection->isOpen()){
            this->connection->close();
        }
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

        boost::asio::ip::tcp::socket socket(* this->ioContext);

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
        if(this->connection.get() == nullptr || !this->connection->isOpen()){
            this->openConnection(logger);
        }
    }

public:
    static bool canSendRequestUnicast(const Node& node) {
        return node.state == NodeState::RUNNING;
    }

    static std::string toJson(const Node& node) {
        return "{\"nodeId\": \""+node.nodeId+"\", \"address\": \""+node.address+"\", \"state\": \""+NodeStates::parseNodeStateToString(node.state)+"\"}";
    }

    static Node fromJson(const nlohmann::json& json) {
        return Node{
                .address = json["address"].get<std::string>(),
                .state = NodeStates::parseNodeStateFromString(json["state"].get<std::string>()),
                .nodeId = json["nodeId"].get<std::string>(),
        };
    }
};