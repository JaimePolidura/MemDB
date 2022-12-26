#pragma once

#include <string>
#include <boost/asio.hpp>

#include "../Users/Authenticator.h"
#include "messages/request/RequestDeserializer.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/ErrorCode.h"
#include "../utils/threads/dynamicthreadpool/DynamicThreadPool.h"
#include "Connection.h"
#include "database/operators/OperatorDispatcher.h"

using namespace boost::asio;

class TCPServer {
private:
    std::shared_ptr<DynamicThreadPool> connectionThreadPool;
    std::shared_ptr<OperatorDispatcher> operatorDispatcher;
    uint16_t port;
    RequestDeserializer requestDeserializer;
    ResponseSerializer responseSerializer;
    Authenticator authenicator;
    io_context ioContext;
    ip::tcp::acceptor acceptator;

public:
    TCPServer(uint16_t port, Authenticator authenicator, std::shared_ptr<OperatorDispatcher> operatorDispatcher, std::shared_ptr<DynamicThreadPool> tcpConnectionThreadPool):
        port(port),
        connectionThreadPool(tcpConnectionThreadPool),
        authenicator(std::move(authenicator)),
        operatorDispatcher(operatorDispatcher),
        acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}) {};

    void run() {
        printf("[SERVER] Waiting for conenctions...\n");

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
            printf("[SERVER] Accepted connection\n");

            std::shared_ptr<Connection> connection = std::make_shared<Connection>(std::move(socket));

            connection->onRequest([connection, this](std::vector<uint8_t> requestRawBuffer) {
                printf("[Server] Enqueued task to connection thread pool\n");

                this->connectionThreadPool->submit([&connection, requestRawBuffer, this] {
                    printf("[SERVER] Found worker. Calling callback\n");
                    this->onNewRequest(requestRawBuffer, connection);
                });
            });

            connection->read(); //Start reading, IO async operation, not blocking

            this->acceptNewConnections();
        });
    }

    void onNewRequest(std::vector<uint8_t> requestRawBuffer, std::shared_ptr<Connection> connection) {
        printf("[SERVER] Deserializing request\n");
        std::shared_ptr<Request> request = this->requestDeserializer.deserialize(requestRawBuffer);
        printf("[SERVER] Authenticating\n");
        bool authenticationValid = this->authenicator.authenticate(request->authentication->authKey);

        if(!authenticationValid){
            printf("[SERVER] Authentication invalid\n");
            this->sendResponse(connection, Response::error(ErrorCode::AUTH_ERROR));
            return;
        }

        printf("[SERVER] Authentication valid\n");

        this->operatorDispatcher->dispatch(request, [this, connection](std::shared_ptr<Response> response){
            printf("[SERVER] Calling onresponse callback %i\n", response->isSuccessful);
            this->sendResponse(connection, response);
        });
    }

    void sendResponse(std::shared_ptr<Connection> connection, std::shared_ptr<Response> response) {
        if(connection->isOpen()){
            std::shared_ptr<std::vector<uint8_t>> serialized = this->responseSerializer.serialize(response);
            connection->write(* serialized);
        }
    }
};