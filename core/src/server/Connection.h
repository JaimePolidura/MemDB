#pragma once

#include "shared.h"
#include "memdbtypes.h"
#include "utils/Utils.h"

#define FRAGMENT_MIN_SIZE 65536

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    ip::tcp::socket socket;

    uint8_t messageLengthHeaderBuffer[sizeof(memDbDataLength_t)];
    uint8_t fragmentationHeaderBuffer[4];
    uint8_t typePacketHeaderBuffer[1];
    
public:
    Connection(ip::tcp::socket socket);

    void onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam);

    std::string getAddress();

    void readAsync();

    std::vector<uint8_t> readSync();

    void writeAsync(const std::vector<uint8_t>& toWrite);

    size_t writeSync(const std::vector<uint8_t>& toWrite);

    bool isOpen();

    void close();

private:
    size_t writeSyncFragmented(const std::vector<uint8_t>& bytes);

    std::vector<uint8_t> readPacket(uint8_t packetType);

    memDbDataLength_t readPacketLength();

    std::vector<uint8_t> readPacketContent();

    std::vector<uint8_t> readFragmentedPacket();

    bool isFragmentPacket(uint8_t packetTypeHeader);
};

using connection_t = std::shared_ptr<Connection>;