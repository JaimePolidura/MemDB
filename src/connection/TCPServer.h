#pragma once

#include <string>
#include <boost/asio.hpp>

#include "../Users/Authenticator.h"
#include "messages/request/RequestDeserializer.h"
#include "../utils/threads/dynamicthreadpool/DynamicThreadPool.h"
#include "TCPConnection.h"

using namespace boost::asio;

class TCPServer {
private:
    std::shared_ptr<DynamicThreadPool> tcpConnectionThreadPool;
    RequestDeserializer requestDeserializer;
    ip::tcp::acceptor acceptator;
    Authenticator authenicator;
    io_context ioContext;
    uint16_t port;

public:
    TCPServer(uint16_t port, Authenticator authenicator, std::shared_ptr<DynamicThreadPool> tcpConnectionThreadPool):
        port(port), authenicator(std::move(authenicator)), acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}),
        tcpConnectionThreadPool(tcpConnectionThreadPool) {};

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

            std::shared_ptr<TCPConnection> connection = std::make_shared<TCPConnection>(std::move(socket));

            connection->read(); //Start reading, IO async operation, not blocking

            connection->onRequest([&](const std::vector<uint8_t>& requestRawBuffer) {
                this->tcpConnectionThreadPool->submit([&] { this->onNewPackage(requestRawBuffer, connection); });
            });

            this->acceptNewConnections();
        });
    }

    void onNewPackage(const std::vector<uint8_t>& requestRawBuffer, const std::shared_ptr<TCPConnection>& connection) {
        std::shared_ptr<Request> request = this->requestDeserializer.deserialize(requestRawBuffer);
        bool authenticationValid = this->authenicator.authenticate(request->authentication->authKey);

        if(!authenticationValid){
            //TODO Send error
            return;
        }

        //TODO Dispatch request
    }
};