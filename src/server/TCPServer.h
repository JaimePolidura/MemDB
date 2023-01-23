#pragma once

#include <string>
#include <boost/asio.hpp>

#include "config/keys/ConfigurationKeys.h"
#include "config/Configuration.h"
#include "../Users/Authenticator.h"
#include "messages/request/RequestDeserializer.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/ErrorCode.h"
#include "../utils/threads/dynamicthreadpool/DynamicThreadPool.h"
#include "Connection.h"
#include "operators/OperatorDispatcher.h"

using namespace boost::asio;

class TCPServer {
private:
    std::shared_ptr<Configuration> configuration;
    DynamicThreadPool connectionThreadPool;
    std::shared_ptr<OperatorDispatcher> operatorDispatcher;
    uint16_t port;
    RequestDeserializer requestDeserializer;
    ResponseSerializer responseSerializer;
    Authenticator authenicator;
    io_context ioContext;
    ip::tcp::acceptor acceptator;

public:
    TCPServer(std::shared_ptr<Configuration> configuration, Authenticator authenicator, std::shared_ptr<OperatorDispatcher> operatorDispatcher):
            configuration(configuration),
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
        Request request = this->requestDeserializer.deserialize(requestRawBuffer);
        bool authenticationValid = this->authenicator.authenticate(request.authentication.authKey);

        if(!authenticationValid){
            const Response errorAuthResponse = Response::error(ErrorCode::AUTH_ERROR, request.requestNumber);
            this->sendResponse(connection, errorAuthResponse);
            return;
        }

        this->operatorDispatcher->dispatch(request, [this, connection](const Response& response){
            this->sendResponse(connection, response);
        });
    }

    void sendResponse(std::shared_ptr<Connection> connection, const Response& response) {
        if(connection->isOpen()){
            std::vector<uint8_t> serialized = this->responseSerializer.serialize(response);
            connection->write(serialized);
        }
    }
};