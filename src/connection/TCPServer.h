#pragma once

#include <string>
#include <boost/asio.hpp>
#include "../Users/Authenticator.h"

using namespace boost::asio;

class TCPServer {
private:
    short unsigned port;
    Authenticator authenicator;
    io_context ioContext;
    ip::tcp::acceptor acceptator;

    static const char flag1Mask = 0x80; //1000 0000
    static const char flag2Mask = 0x40; //0100 0000

public:
    TCPServer(short unsigned port, Authenticator authenicator): port(port), authenicator(std::move(authenicator)),
                                                                acceptator(ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}) {};

    void run() {
        while(true) {
            this->waitForConnectoins();
        }
    }

private:
    void waitForConnectoins() {
        this->acceptator.async_accept([this](std::error_code ec, boost::asio::ip::tcp::socket socket) {
            this->handleNewConnection(socket);
            this->waitForConnectoins();
        });
    }

    void handleNewConnection(boost::asio::ip::tcp::socket& socket) {
        //TODO Proceed
    }
};