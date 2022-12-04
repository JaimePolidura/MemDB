#pragma once

#include <string>
#include <boost/asio.hpp>
#include <utility>
#include "../Users/Authenticator.h"

using namespace boost::asio;

class TCPServer {
private:
    short unsigned port;
    Authenticator authenicator;

    static const char flag1Mask = 0x80; //1000 0000
    static const char flag2Mask = 0x40; //0100 0000

public:
    TCPServer(short unsigned port, Authenticator authenicator): port(port), authenicator(std::move(authenicator)) {};

    void run() {
        io_context ioContext{};

        ip::tcp::acceptor acceptor {ioContext, ip::tcp::endpoint{ip::tcp::v4(), this->port}};

        while(true) {
            ip::tcp::socket socket{ioContext};
            acceptor.accept(socket);
            handle(socket);
        }
    }

private:
    void handle(boost::asio::ip::tcp::socket& socket) {
        boost::system::error_code ec;
        size_t bytes = socket.available();
        std::vector<char> vBuffer(bytes);

        socket.read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()), ec);

        unsigned short authHeaderLength = (unsigned short) vBuffer[0] >> 2;
        bool flag1 = vBuffer[0] << 6 & flag1Mask;
        bool flag2 = vBuffer[0] << 6 & flag2Mask;

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
};