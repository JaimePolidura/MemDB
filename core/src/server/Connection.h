#pragma once

#include "shared.h"

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    uint8_t requestLengthBuffer[sizeof(defaultMemDbRequestLength_t)];
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    ip::tcp::socket socket;
public:
    AuthenticationType authenticationType;

    explicit Connection(ip::tcp::socket socket) : socket{std::move(socket)} {}

    void onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam) {
        this->onRequestCallback = onRequestCallbackParam;
    }

    std::string getAddress() {
        auto ip = this->socket.remote_endpoint().address().to_string();
        auto port = this->socket.remote_endpoint().port();

        return ip + ":" + std::to_string(port);
    }

    void read() {
        std::shared_ptr<Connection> self = shared_from_this();

        this->socket.async_read_some(boost::asio::buffer(requestLengthBuffer, sizeof(defaultMemDbRequestLength_t)), [this, self](boost::system::error_code ec, std::size_t lengthRead){
            if(ec) return;

            auto requestLength = Utils::parse<defaultMemDbRequestLength_t>(requestLengthBuffer);

            std::vector<uint8_t> requestBuffer(requestLength);

            this->socket.read_some(boost::asio::buffer(requestBuffer));

            this->onRequestCallback(requestBuffer);

            this->read();
        });
    }

    void write(const std::vector<uint8_t>& toWrite) {
        this->socket.async_write_some(boost::asio::buffer(toWrite), [&](const boost::system::error_code& error, std::size_t bytes_transferred){});
    }

    bool isOpen() {
        return this->socket.is_open();
    }
};