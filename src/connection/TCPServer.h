#pragma once

#include <string>
#include <boost/asio.hpp>

#include "../Users/Authenticator.h"
#include "messages/request/RequestDeserializer.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/ErrorCodes.h"
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

            connection->onRequest([&](const std::vector<uint8_t>& requestRawBuffer) {
                this->connectionThreadPool->submit([&] { this->onNewPackage(requestRawBuffer, connection); });
            });

            connection->read(); //Start reading, IO async operation, not blocking

            this->acceptNewConnections();
        });
    }

    void onNewPackage(const std::vector<uint8_t>& requestRawBuffer, const std::shared_ptr<Connection>& connection) {
        std::shared_ptr<Request> request = this->requestDeserializer.deserialize(requestRawBuffer);
        bool authenticationValid = this->authenicator.authenticate(request->authentication->authKey);

        if(!authenticationValid){
            const Response &authErrorResponse = std::move(Response::error(AUTH_ERROR));
            connection->write(* this->responseSerializer.serialize(authErrorResponse));
            return;
        }

        this->operatorDispatcher->dispatch(request, connection, [this, &connection](std::shared_ptr<Response> response){
            this->onResponseFromDb(connection, response);
        });
    }

    void onResponseFromDb(const std::shared_ptr<Connection>& connection, std::shared_ptr<Response> response) {
        if(connection->isOpen()){
            std::shared_ptr<std::vector<uint8_t>> serialized = this->responseSerializer.serialize(* response);
            connection->write(* serialized);
        }
    }
};