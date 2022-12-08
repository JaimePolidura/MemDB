#pragma once

#include <string>
#include <boost/asio.hpp>
#include "../Users/Authenticator.h"
#include "../Messages/MessageParser.h"
#include "TCPConnection.h"

using namespace boost::asio;

class TCPServer {
private:
    short unsigned port;
    Authenticator authenicator;
    io_context ioContext;
    ip::tcp::acceptor acceptator;

public:
    TCPServer(short unsigned port, Authenticator authenicator): port(port), authenicator(std::move(authenicator)),
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

            std::shared_ptr<TCPConnection> connection = std::make_shared<TCPConnection>(std::move(socket));

            connection->read();

            this->acceptNewConnections();
        });
    }
};