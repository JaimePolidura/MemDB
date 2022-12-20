#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <iostream>
#include <functional>

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    uint8_t requestBuffer[1500];
    ip::tcp::socket socket;

public:
    explicit Connection(ip::tcp::socket socket) : socket{std::move(socket)} {}

    void onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam) {
        this->onRequestCallback = onRequestCallbackParam;
    }

    void read() {
        std::shared_ptr<Connection> self = shared_from_this();

        this->socket.async_read_some(boost::asio::buffer(requestBuffer, 1500), [this, self](boost::system::error_code ec, std::size_t length){
            if(ec) return;

            printf("[SERVER] Read request with bytes: %llu\n", length);

            std::vector<uint8_t> vectorBuffer(this->requestBuffer, this->requestBuffer + length);

            this->onRequestCallback(vectorBuffer);

            this->read();
        });
    }

    void write(const std::vector<uint8_t>& toWrite) {
        this->socket.async_write_some(boost::asio::buffer(toWrite),  [&](const boost::system::error_code& error, std::size_t bytes_transferred){});
    }

    bool isOpen() {
        return this->socket.is_open();
    }
};