#pragma once

#include "shared.h"
#include "memdbtypes.h"
#include "utils/Utils.h"
#include "logging/Logger.h"

#define FRAGMENT_MIN_SIZE 65536

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    ip::tcp::socket socket;
    logger_t logger;

    uint8_t messageLengthHeaderBuffer[sizeof(memDbDataLength_t)];
    uint8_t fragmentationHeaderBuffer[4];
    uint8_t typePacketHeaderBuffer[1];
    
public:
    Connection(ip::tcp::socket socket, logger_t logger);

    void onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam);

    std::string getAddress();

    void readAsync();

    std::vector<uint8_t> readSync();

    void writeAsync(std::vector<uint8_t>& toWrite);

    std::size_t writeSync(std::vector<uint8_t>& toWrite);

    bool isOpen();

    void close();

private:
    std::size_t writeSyncFragmented(std::vector<uint8_t>& bytes);

    std::vector<uint8_t> readPacket(uint8_t packetType);

    std::size_t fragmentPacketAndSend(std::vector<uint8_t>& packet, std::function<std::size_t(std::vector<uint8_t>&)> sender);

    std::vector<uint8_t> addNoFragmentationHeader(std::vector<uint8_t>& vec);

    memDbDataLength_t readPacketLength();

    std::vector<uint8_t> readPacketContent();

    std::vector<uint8_t> readFragmentedPacket();

    bool isFragmentPacket(uint8_t packetTypeHeader);
};

using connection_t = std::shared_ptr<Connection>;