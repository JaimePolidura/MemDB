#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <iostream>

using namespace boost::asio;

class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
private:
    ip::tcp::socket socket;
    uint8_t messageBuffer[1500];
    MessageParser messageParser;

public:
    explicit TCPConnection(ip::tcp::socket socket) : socket{std::move(socket)} {}

    void read() {
        std::shared_ptr<TCPConnection> self = shared_from_this();

        this->socket.async_read_some(boost::asio::buffer(messageBuffer, 1500), [this, self](boost::system::error_code ec, std::size_t length){
            if(ec) return;

            printf("[SERVER] Read message with bytes: %llu\n", length);

            std::vector<uint8_t> vectorBuffer(this->messageBuffer, this->messageBuffer + length);

            std::shared_ptr<Message> message = this->messageParser.parse(vectorBuffer);

            this->read();
        });
    }
};