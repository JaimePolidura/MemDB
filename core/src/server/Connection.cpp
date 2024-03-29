#include "server/Connection.h"

Connection::Connection(ip::tcp::socket socket, logger_t logger) : socket{std::move(socket)}, logger(logger) {
    this->enableTcpNoDelay();
}

void Connection::onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam) {
    this->onRequestCallback = onRequestCallbackParam;
}

void Connection::setResetConnection(std::function<ip::tcp::socket()> resetCallback) {
    this->resetConnectionCallback = resetCallback;
}

bool Connection::isOpen() {
    return this->socket.is_open();
}

void Connection::close() {
    return this->socket.close();
}

std::string Connection::getAddress() {
    auto ip = this->socket.remote_endpoint().address().to_string();
    auto port = this->socket.remote_endpoint().port();

    return ip + ":" + std::to_string(port);
}

void Connection::readAsync(uint64_t timeout) {
    std::shared_ptr<Connection> self = shared_from_this();

    this->socket.async_read_some(boost::asio::buffer(this->typePacketHeaderBuffer, sizeof(this->typePacketHeaderBuffer)),
                                 [this, self, timeout](boost::system::error_code ec, std::size_t lengthRead){
        if(ec) return;


        std::result<std::vector<uint8_t>> packetReadResult = this->readPacket(this->typePacketHeaderBuffer[0], timeout);

        if(packetReadResult.is_success()){
            this->onRequestCallback(packetReadResult.get());
        }

        this->readAsync(timeout);
    });
}

std::result<std::vector<uint8_t>> Connection::readSync(uint64_t ms) {
    if(this->readWithTimeout(boost::asio::buffer(this->typePacketHeaderBuffer), ms)){
        return std::error<std::vector<uint8_t>>();
    }

    return this->readPacket(this->typePacketHeaderBuffer[0], ms);
}

std::result<std::vector<uint8_t>> Connection::readPacket(uint8_t packetType, uint64_t timeoutMs) {
    if(this->isFragmentPacket(packetType)){
        return this->readFragmentedPacket(timeoutMs);
    } else {
        return this->readPacketContent(timeoutMs);
    }
}

std::result<std::vector<uint8_t>> Connection::readPacketContent(uint64_t timeoutMs) {
    if(this->readWithTimeout(boost::asio::buffer(this->messageLengthHeaderBuffer), timeoutMs)){
        return std::error<std::vector<uint8_t>>();
    }

    memDbDataLength_t packetLength = Utils::parse<memDbDataLength_t>(messageLengthHeaderBuffer);

    if(packetLength == 0) {
        return readPacketContent(timeoutMs);
    }

    std::vector<uint8_t> messageBuffer(packetLength);
    if(this->readWithTimeout(boost::asio::buffer(messageBuffer), timeoutMs)){
        return std::error<std::vector<uint8_t>>();
    }

    return std::ok(messageBuffer);
}

std::result<std::vector<uint8_t>> Connection::readFragmentedPacket(uint64_t timeoutMs) {
    std::vector<uint8_t> result{};
    int fragmentsReceived = 0;

    this->logger->debugInfo("Received fragmented packet, starting to read");

    this->setTcpReceiveBufferSize(65565 * 8);

    while(true) {
        fragmentsReceived++;

        std::result<std::vector<uint8_t>> packetResult = this->readPacketContent(timeoutMs);
        if(packetResult.has_error()){
            return packetResult;
        }

        result.insert(result.end(), packetResult->begin(), packetResult->end());

        this->logger->debugInfo("Received fragment {0} packet of {1} kb", fragmentsReceived, packetResult->size() / 1024);

        if(this->isLastFragmentPacket(this->typePacketHeaderBuffer[0])){
            this->setTcpReceiveBufferSize(65565);
            return std::ok(result);
        }

        if(this->readWithTimeout(boost::asio::buffer(this->typePacketHeaderBuffer), timeoutMs)){
            return std::error<std::vector<uint8_t>>();
        }
    }
}

void Connection::writeAsync(std::vector<uint8_t>& toWrite) {
    if(toWrite.size() < FRAGMENT_MIN_SIZE){
        this->addContentLengthHeader(toWrite, toWrite.size());
        this->addNoFragmentationHeader(toWrite);

        this->socket.async_write_some(boost::asio::buffer(toWrite), [](const boost::system::error_code& error, std::size_t bytes_transferred){});
    } else {
        this->fragmentPacketAndSend(toWrite, [this](std::vector<uint8_t>& fragmentedPacket){
            this->socket.async_write_some(boost::asio::buffer(fragmentedPacket), [](const boost::system::error_code& error, std::size_t bytes_transferred){});
            return 0;
        });
    }
}

size_t Connection::writeSync(std::vector<uint8_t>& toWrite) {
    if(toWrite.size() < FRAGMENT_MIN_SIZE){
        this->addContentLengthHeader(toWrite, toWrite.size());
        this->addNoFragmentationHeader(toWrite);

        std::size_t written = this->socket.write_some(boost::asio::buffer(toWrite));

        if(written == 0){
            this->resetConnection();
            return this->socket.write_some(boost::asio::buffer(toWrite));
        } else {
            return written;
        }
    } else {
        return this->fragmentPacketAndSend(toWrite, [this](std::vector<uint8_t>& fragmentedPacket){
            return this->socket.write_some(boost::asio::buffer(fragmentedPacket));
        });
    }
}

std::size_t Connection::fragmentPacketAndSend(std::vector<uint8_t>& initialPacket, std::function<std::size_t(std::vector<uint8_t>&)> sender) {
    memDbDataLength_t contentSizeInFragment = initialPacket.size() < FRAGMENT_MIN_SIZE ? initialPacket.size() : FRAGMENT_MIN_SIZE;
    int nFragments = initialPacket.size() / FRAGMENT_MIN_SIZE + (initialPacket.size() % FRAGMENT_MIN_SIZE == 0 ? 0 : 1);
    uint8_t * startFromInitialPacket = (uint8_t *) initialPacket.data();
    size_t written = 0;

    this->logger->debugInfo("Starting fragmentation to send a packet of {0} kb with {1} fragments", initialPacket.size() / 1024, nFragments);

    this->enableTcpNoDelay();
    this->setTcpSendBufferSize(1090);

    for(int i = 0; i < nFragments; i++) {
        std::vector<uint8_t> fragmentedPacket{};
        bool lastPacket = i + 1 >= nFragments;
        contentSizeInFragment = lastPacket ? initialPacket.size() % FRAGMENT_MIN_SIZE : FRAGMENT_MIN_SIZE;

        Utils::appendBeginningToBuffer(contentSizeInFragment, fragmentedPacket); //Content length
        if(!lastPacket){
            Utils::appendBeginningToBuffer<uint8_t>(0x01, fragmentedPacket); //Is fragmented flag
        }
        if(lastPacket) {
            Utils::appendBeginningToBuffer<uint8_t>(0x03, fragmentedPacket); //Last fragment
        }

        fragmentedPacket.insert(fragmentedPacket.begin() + 1 + sizeof(memDbDataLength_t), startFromInitialPacket, startFromInitialPacket + contentSizeInFragment);

        this->logger->debugInfo("Sending fragment packet {0} of {1} kb", i + 1, fragmentedPacket.size() / 1024);
        std::this_thread::sleep_for(std::chrono::milliseconds(25)); //TODO Fix TCP Windows issue
        written += sender(fragmentedPacket);

        if(!lastPacket) {
            startFromInitialPacket += contentSizeInFragment;
        }
    }

    this->setTcpSendBufferSize(65565);

    return written;
}

std::vector<uint8_t> Connection::addContentLengthHeader(std::vector<uint8_t>& vec, memDbDataLength_t contentLength) {
    Utils::appendBeginningToBuffer(contentLength, vec);
    return vec;
}

std::vector<uint8_t> Connection::addNoFragmentationHeader(std::vector<uint8_t>& vec) {
    vec.insert(vec.begin(), 0x00);
    return vec;
}

bool Connection::isFragmentPacket(uint8_t packetTypeHeader) {
    return (packetTypeHeader & 0x01) != 0;
}

bool Connection::isLastFragmentPacket(uint8_t packetTypeHeader) {
    return (packetTypeHeader & 0x03) == 0x03;
}

void Connection::setTcpReceiveBufferSize(std::size_t size) {
    boost::asio::socket_base::receive_buffer_size receiveBufferSize(size);
    this->socket.set_option(receiveBufferSize);
}

void Connection::enableTcpNoDelay() {
    boost::asio::ip::tcp::no_delay option(true);
    this->socket.set_option(option);
}

void Connection::setTcpSendBufferSize(std::size_t size) {
    boost::asio::socket_base::send_buffer_size sendBuffer(size);
    this->socket.set_option(sendBuffer);
}

boost::system::error_code Connection::readWithTimeout(boost::asio::mutable_buffers_1 buffer, uint64_t timeoutMs) {
    boost::system::error_code errorCode;

    boost::asio::read(this->socket, buffer, errorCode);

    return errorCode;
}

void Connection::resetConnection() {
    this->close();
    if(this->resetConnectionCallback != nullptr){
        this->socket = this->resetConnectionCallback();
    }
}