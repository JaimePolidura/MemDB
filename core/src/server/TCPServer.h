#pragma once

#include <string>
#include <boost/asio.hpp>

#include "config/keys/ConfigurationKeys.h"
#include "config/Configuration.h"
#include "auth/Authenticator.h"
#include "messages/request/RequestDeserializer.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/ErrorCode.h"
#include "utils/threads/pool/DynamicThreadPool.h"
#include "Connection.h"
#include "operators/OperatorDispatcher.h"

using namespace boost::asio;

class TCPServer {
private:
    operatorDispatcher_t operatorDispatcher;
    configuration_t configuration;
    DynamicThreadPool connectionThreadPool;
    uint16_t port;
    RequestDeserializer requestDeserializer;
    ResponseSerializer responseSerializer;
    Authenticator authenicator;
    io_context ioContext;
    ip::tcp::acceptor acceptator;
    replication_t replication;

public:
    TCPServer(configuration_t configuration, replication_t replication, Authenticator authenicator, operatorDispatcher_t operatorDispatcher):
            configuration(configuration),
            replication(replication),
            port(configuration->get<uint16_t>(ConfigurationKeys::PORT)),
            authenicator(std::move(authenicator)),
            connectionThreadPool(5, configuration->get<int>(ConfigurationKeys::SERVER_MAX_THREADS), configuration->get<int>(ConfigurationKeys::SERVER_MIN_THREADS), 100, "TCPServer"),
            operatorDispatcher(operatorDispatcher),
            acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}) {};

    void run() {
        printf("[SERVER] Initialized. Waiting for conenctions...\n");

        try{
            this->acceptNewConnections();

            this->ioContext.run();
        }catch (const std::exception& e) {
            printf("[SERVER] fatal error %s\n", e.what());
        }
    }

private:
    void acceptNewConnections() {
        this->acceptator.async_accept([this](std::error_code ec, ip::tcp::socket socket) {
            std::shared_ptr<Connection> connection = std::make_shared<Connection>(std::move(socket));

            connection->onRequest([connection, this](const std::vector<uint8_t>& requestRawBuffer) {
                this->connectionThreadPool.submit([connection, requestRawBuffer, this] {
                    this->onNewRequest(requestRawBuffer, connection);
                });
            });

            connection->read(); //Start reading, IO async operation, not blocking

            this->acceptNewConnections();
        });
    }
    
    void onNewRequest(const std::vector<uint8_t>& requestRawBuffer, std::shared_ptr<Connection> connection) {
        bool isConnectionFromReplicaNode = this->isConnectionFromReplicaNode(connection);

        Request request = this->requestDeserializer.deserialize(requestRawBuffer, isConnectionFromReplicaNode);
        bool authenticationValid = this->authenicator.authenticate(request.authentication.authKey);

        if(!authenticationValid){
            const Response errorAuthResponse = Response::error(ErrorCode::AUTH_ERROR, request.requestNumber, request.operation.timestamp);
            this->sendResponse(connection, errorAuthResponse);
            return;
        }

        AuthenticationType authenticationType = this->authenicator.getAuthenticationType(request.authentication.authKey);
        request.authenticationType = authenticationType;
        connection->authenticationType = authenticationType;

        Response response = this->operatorDispatcher->dispatch(request);
        response.requestNumber = request.requestNumber;

        this->sendResponse(connection, response);
    }

    void sendResponse(std::shared_ptr<Connection> connection, const Response& response) {
        if(connection->isOpen()){
            std::vector<uint8_t> serialized = this->responseSerializer.serialize(response);
            connection->write(serialized);
        }
    }

    bool isConnectionFromReplicaNode(std::shared_ptr<Connection> connection) {
        return this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION) &&
            this->replication->doesBelongToReplicationNode(connection->getAddress());
    }
};

using tcpServer_t = std::shared_ptr<TCPServer>;