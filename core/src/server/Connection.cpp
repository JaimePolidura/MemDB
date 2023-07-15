#include "server/Connection.h"

Connection::Connection(ip::tcp::socket socket) : socket{std::move(socket)} {}

void Connection::onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam) {
    this->onRequestCallback = onRequestCallbackParam;
}

std::string Connection::getAddress() {
    auto ip = this->socket.remote_endpoint().address().to_string();
    auto port = this->socket.remote_endpoint().port();

    return ip + ":" + std::to_string(port);
}

void Connection::readAsync() {
    std::shared_ptr<Connection> self = shared_from_this();

    this->socket.async_read_some(boost::asio::buffer(messageLengthBuffer, sizeof(memDbDataLength_t)), [this, self](boost::system::error_code ec, std::size_t lengthRead){
        if(ec) return;

        memDbDataLength_t messageLength = Utils::parse<memDbDataLength_t>(messageLengthBuffer);

        std::vector<uint8_t> messageBuffer = this->readSocketBufferLenght(messageLength);

        this->onRequestCallback(messageBuffer);

        this->readAsync();
    });
}

std::vector<uint8_t> Connection::readSync() {
    boost::asio::read(this->socket, boost::asio::buffer(this->messageLengthBuffer));
    memDbDataLength_t messageLength = Utils::parse<memDbDataLength_t>(messageLengthBuffer);

    return this->readSocketBufferLenght(messageLength);
}

void Connection::writeAsync(const std::vector<uint8_t>& toWrite) {
    this->socket.async_write_some(boost::asio::buffer(toWrite), [&](const boost::system::error_code& error, std::size_t bytes_transferred){});
}

size_t Connection::writeSync(const std::vector<uint8_t>& toWrite) {
    return this->socket.write_some(boost::asio::buffer(toWrite));
}

bool Connection::isOpen() {
    return this->socket.is_open();
}

void Connection::close() {
    return this->socket.close();
}

std::vector<uint8_t> Connection::readSocketBufferLenght(memDbDataLength_t length) {
    std::vector<uint8_t> messageBuffer(length);
    this->socket.read_some(boost::asio::buffer(messageBuffer));

    return messageBuffer;
}