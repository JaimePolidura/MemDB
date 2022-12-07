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
        boost::system::error_code ec;
        size_t bytes = socket.available();
        std::vector<char> vBuffer(bytes);

        socket.read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()), ec);

        unsigned short authHeaderLength = (unsigned short) vBuffer[0] >> 2;

        char authKeyBuffer [authHeaderLength];
        for(int i = 0; i < authHeaderLength; i++)
            authKeyBuffer[i] = vBuffer[i + 1];

        bool authenticationResult = this->authenicator.authenticate(authKeyBuffer);
        if(!authenticationResult){
            socket.close(ec);
            return;
        }

        //TODO Proceed
    }

    unsigned short getLength(const std::vector<char>& buffer, int pos) {
        return (unsigned short) buffer[pos] >> 2;
    }

    bool getFlag(const std::vector<char>& buffer, int pos, char flagMask) {
        return (buffer[pos] << 6) & flagMask;
    }
};