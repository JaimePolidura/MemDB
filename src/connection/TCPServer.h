#pragma once

#include <string>
#include <boost/asio.hpp>
#include "../Users/Authenticator.h"
#include "../Messages/MessageParser.h"

using namespace boost::asio;

class TCPServer {
private:
    short unsigned port;
    Authenticator authenicator;
    io_context ioContext;
    ip::tcp::acceptor acceptator;
    MessageParser messageParser;

public:
    TCPServer(short unsigned port, Authenticator authenicator): port(port), authenicator(std::move(authenicator)),
                                                                acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}) {};

    void run() {
        printf("[Server] Waiting for conenctions...\n");

        try{
            this->waitForConnectoins();

            this->ioContext.run();
        }catch (std::exception& e) {
            printf("[Server] fatal error\n");
        }
    }

private:
    void waitForConnectoins() {
        this->acceptator.async_accept([this](std::error_code ec, ip::tcp::socket socket) {
            printf("[Server] Accepted connection\n");

            size_t bytes = socket.available();
            std::vector<uint8_t> buffer(bytes);

            socket.async_read_some(boost::asio::buffer(buffer), [this](const boost::system::error_code& error, std::size_t bytesRead) {
                printf("[Server] Read %llu bytes\n", bytesRead);
            });

            this->waitForConnectoins();
        });
    }
};