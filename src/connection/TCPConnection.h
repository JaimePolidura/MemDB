#pragma once

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <iostream>
#include <functional>

using namespace boost::asio;

class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
private:
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    uint8_t requestBuffer[1500];
    ip::tcp::socket socket;

public:
    explicit TCPConnection(ip::tcp::socket socket) : socket{std::move(socket)} {}

    void onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam) {
        this->onRequestCallback = onRequestCallbackParam;
    }

    void read() {
        std::shared_ptr<TCPConnection> self = shared_from_this();

        this->socket.async_read_some(boost::asio::buffer(requestBuffer, 1500), [this, self](boost::system::error_code ec, std::size_t length){
            if(ec) return;

            printf("[SERVER] Read request with bytes: %llu\n", length);

            std::vector<uint8_t> vectorBuffer(this->requestBuffer, this->requestBuffer + length);

            this->onRequestCallback(vectorBuffer);

            this->read();
        });
    }
};