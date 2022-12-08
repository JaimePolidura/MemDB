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
    MessageParser messageParser;

public:
    TCPServer(short unsigned port, Authenticator authenicator): port(port), authenicator(std::move(authenicator)),
                                                                acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}) {};

    void run() {
        printf("[Server] Waiting for conenctions...\n");

        try{
            this->waitForConnectoins();

            this->ioContext.run();
        }catch (const std::exception& e) {
            printf("[Server] fatal error %s\n", e.what());
        }
    }

private:
    void waitForConnectoins() {
        this->acceptator.async_accept([this](std::error_code ec, ip::tcp::socket socket) {
            printf("[Server] Accepted connection\n");

            std::shared_ptr<TCPConnection> connection = std::make_shared<TCPConnection>(std::move(socket));

            connection->read();

//            socket.async_read_some(boost::asio::buffer(buffer), [&buffer](const boost::system::error_code& ec, std::size_t bytesRead) {
//                if(!ec && ec != boost::asio::error::eof){
//                    printf("[Server] Read %llu bytes\n", bytesRead);
//                    printf("[Server] Read message: %s\n", buffer.data());
//                }
//            });

            this->waitForConnectoins();
        });
    }
};