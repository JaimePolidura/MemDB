#pragma once

#include "shared.h"

#include "config/keys/ConfigurationKeys.h"
#include "config/Configuration.h"
#include "auth/Authenticator.h"
#include "messages/request/RequestDeserializer.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/ErrorCode.h"
#include "utils/threads/pool/DynamicThreadPool.h"
#include "server/Connection.h"
#include "operators/OperatorDispatcher.h"
#include "logging/Logger.h"

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
    logger_t logger;

public:
    TCPServer(logger_t logger, configuration_t configuration, replication_t replication, Authenticator authenicator, operatorDispatcher_t operatorDispatcher):
            configuration(configuration),
            logger(logger),
            replication(replication),
            port(configuration->get<uint16_t>(ConfigurationKeys::MEMDB_CORE_PORT)),
            authenicator(std::move(authenicator)),
            connectionThreadPool(5, configuration->get<int>(ConfigurationKeys::MEMDB_CORE_SERVER_MAX_THREADS), configuration->get<int>(ConfigurationKeys::MEMDB_CORE_SERVER_MIN_THREADS), 100),
            operatorDispatcher(operatorDispatcher),
            acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}) {};

    void run() {
        this->logger->info("Listening for conenctions...");

        try{
            this->acceptNewConnections();

            this->ioContext.run();
        }catch (const std::exception& e) {
            this->logger->error("Fatal error in TCP Server {0}", e.what());
        }
    }

private:
    void acceptNewConnections() {
        this->acceptator.async_accept([this](std::error_code ec, ip::tcp::socket socket) {
            std::shared_ptr<Connection> connection = std::make_shared<Connection>(std::move(socket));

            this->logger->debugInfo("Accepted TCP Connection {0}", connection->getAddress());

            connection->onRequest([connection, this](const std::vector<uint8_t>& requestRawBuffer) {
                this->connectionThreadPool.submit([connection, requestRawBuffer, this] {
                    this->onNewRequest(requestRawBuffer, connection);
                });
            });

            connection->readAsync(); //Start reading, IO async operation, not blocking

            this->acceptNewConnections();
        });
    }

    void onNewRequest(const std::vector<uint8_t>& requestRawBuffer, connection_t connection) {
        Request request = this->requestDeserializer.deserialize(requestRawBuffer);
        bool authenticationValid = this->authenicator.authenticate(request.authentication.authKey);

        if(!authenticationValid){
            this->sendAuthError(connection, request);
            return;
        }

        request.authenticationType = this->authenicator.getAuthenticationType(request.authentication.authKey);
        if(request.authenticationType != AuthenticationType::NODE && request.authentication.flag1){//Only node id can be present in request with node keys
            this->sendAuthError(connection, request);
            return;
        }

        if(request.authenticationType == AuthenticationType::NODE){
            this->addClusterConnectionIfNotAdded(request, connection);
        }

        Response response = this->operatorDispatcher->dispatch(request);
        response.requestNumber = request.requestNumber;

        this->sendResponse(connection, response);
    }

    void sendAuthError(connection_t connection, Request request) {
        const Response errorAuthResponse = Response::error(ErrorCode::AUTH_ERROR, request.requestNumber, request.operation.timestamp);
        this->sendResponse(connection, errorAuthResponse);
    }

    void sendResponse(std::shared_ptr<Connection> connection, const Response& response) {
        if(connection->isOpen()){
            std::vector<uint8_t> serialized = this->responseSerializer.serialize(response);
            connection->writeAsync(serialized);
        }
    }

    void addClusterConnectionIfNotAdded(Request request, connection_t connection) {
        if(!this->replication->getClusterNodes()->isConnectionOpened(request.operation.nodeId)){
            this->replication->getClusterNodes()->setConnectionOfNode(request.operation.nodeId, connection);
        }
    }
};

using tcpServer_t = std::shared_ptr<TCPServer>;