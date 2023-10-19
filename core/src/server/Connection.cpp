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

    this->socket.async_read_some(boost::asio::buffer(this->typePacketHeaderBuffer, sizeof(this->typePacketHeaderBuffer)), [this, self](boost::system::error_code ec, std::size_t lengthRead){
        if(ec) return;
        
        this->onRequestCallback(this->readPacket(this->typePacketHeaderBuffer[0]));

        this->readAsync();
    });
}

std::vector<uint8_t> Connection::readSync() {
    boost::asio::read(this->socket, boost::asio::buffer(this->typePacketHeaderBuffer));
    return this->readPacket(this->typePacketHeaderBuffer[0]);
}

std::vector<uint8_t> Connection::readFragmentedPacket() {
    std::vector<uint8_t> result{};

    while(true) {
        boost::asio::read(this->socket, boost::asio::buffer(this->fragmentationHeaderBuffer));
        if(Utils::parse<int>(this->fragmentationHeaderBuffer) < 0){ //nFragment
            return result;
        }

        std::vector<uint8_t> packet = this->readPacketContent();
        result.insert(result.begin(), packet.begin(), packet.end());
        boost::asio::read(this->socket, boost::asio::buffer(this->typePacketHeaderBuffer));
    }
}

void Connection::writeAsync(const std::vector<uint8_t>& toWrite) {
    this->socket.async_write_some(boost::asio::buffer(toWrite), [&](const boost::system::error_code& error, std::size_t bytes_transferred){});
}

size_t Connection::writeSync(const std::vector<uint8_t>& toWrite) {
    if(toWrite.size() < 65536){
        return this->socket.write_some(boost::asio::buffer(toWrite));
    } else {
        return this->writeSyncFragmented(toWrite);
    }
}

std::vector<uint8_t> Connection::readPacket(uint8_t packetType) {
    if(this->isFragmentPacket(packetType)){
        return readFragmentedPacket();
    } else {
        return this->readPacketContent();
    }
}

size_t Connection::writeSyncFragmented(const std::vector<uint8_t>& bytes) {
    int nFragments = bytes.size() / FRAGMENT_MIN_SIZE + (bytes.size() % FRAGMENT_MIN_SIZE == 0 ? 0 : 1);
    uint8_t * start = (uint8_t *) bytes.data();
    int offset = bytes.size() < FRAGMENT_MIN_SIZE ? bytes.size() : FRAGMENT_MIN_SIZE;
    size_t written = 0;
    
    for(int i = 0; i < nFragments; i++){
        std::vector<uint8_t> fragmentedPacket{};
        fragmentedPacket.reserve(sizeof(int) + bytes.size() + 1);
        int nFragment = nFragments - i - 1;

        Utils::appendToBuffer<uint8_t>(0x01, fragmentedPacket); //Is fragmented flag
        Utils::appendToBuffer<int>(nFragment, fragmentedPacket); //Nº Fragment

        fragmentedPacket.insert(fragmentedPacket.begin() + sizeof(int) + 1, start, start + offset);

        written += this->socket.write_some(boost::asio::buffer(fragmentedPacket));

        offset = bytes.size() < 65536 ? bytes.size() : 65536;
        start += offset;
    }

    return written;
}

std::vector<uint8_t> Connection::readPacketContent() {
    boost::asio::read(this->socket, boost::asio::buffer(this->messageLengthHeaderBuffer));
    std::vector<uint8_t> messageBuffer(Utils::parse<memDbDataLength_t>(messageLengthHeaderBuffer));
    this->socket.read_some(boost::asio::buffer(messageBuffer));

    return messageBuffer;
}

bool Connection::isOpen() {
    return this->socket.is_open();
}

bool Connection::isFragmentPacket(uint8_t packetTypeHeader) {
    return (packetTypeHeader & 0x01) == 0;
}

void Connection::close() {
    return this->socket.close();
}