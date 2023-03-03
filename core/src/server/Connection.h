#pragma once

#include <memory>
#include <boost/asio.hpp>

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    uint8_t requestBuffer[1500];
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

        this->socket.async_read_some(boost::asio::buffer(requestBuffer, 1500), [this, self](boost::system::error_code ec, std::size_t length){
            if(ec) return;

            std::vector<uint8_t> vectorBuffer(this->requestBuffer, this->requestBuffer + length);

            this->onRequestCallback(vectorBuffer);

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