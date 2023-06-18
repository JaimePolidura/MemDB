#pragma once

#include "shared.h"
#include "memdbtypes.h"

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    uint8_t messageLengthBuffer[sizeof(memDbDataLength_t)];
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    ip::tcp::socket socket;

public:
    Connection(ip::tcp::socket socket) : socket{std::move(socket)} {}

    void onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam) {
        this->onRequestCallback = onRequestCallbackParam;
    }

    std::string getAddress() {
        auto ip = this->socket.remote_endpoint().address().to_string();
        auto port = this->socket.remote_endpoint().port();

        return ip + ":" + std::to_string(port);
    }

    void readAsync() {
        std::shared_ptr<Connection> self = shared_from_this();

        this->socket.async_read_some(boost::asio::buffer(messageLengthBuffer, sizeof(memDbDataLength_t)), [this, self](boost::system::error_code ec, std::size_t lengthRead){
            if(ec) return;

            memDbDataLength_t messageLength = Utils::parse<memDbDataLength_t>(messageLengthBuffer);

            std::vector<uint8_t> messageBuffer = this->readSocketBufferLenght(messageLength);

            this->onRequestCallback(messageBuffer);

            this->readAsync();
        });
    }

    std::vector<uint8_t> readSync() {
        boost::asio::read(this->socket, boost::asio::buffer(this->messageLengthBuffer));
        memDbDataLength_t messageLength = Utils::parse<memDbDataLength_t>(messageLengthBuffer);

        return this->readSocketBufferLenght(messageLength);
    }

    void writeAsync(const std::vector<uint8_t>& toWrite) {
        this->socket.async_write_some(boost::asio::buffer(toWrite), [&](const boost::system::error_code& error, std::size_t bytes_transferred){});
    }

    size_t writeSync(const std::vector<uint8_t>& toWrite) {
        return this->socket.write_some(boost::asio::buffer(toWrite));
    }

    bool isOpen() {
        return this->socket.is_open();
    }

    void close() {
        return this->socket.close();
    }

private:
    std::vector<uint8_t> readSocketBufferLenght(memDbDataLength_t length) {
        std::vector<uint8_t> messageBuffer(length);
        this->socket.read_some(boost::asio::buffer(messageBuffer));

        return messageBuffer;
    }
};

using connection_t = std::shared_ptr<Connection>;