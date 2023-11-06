#pragma once

#include "shared.h"
#include "memdbtypes.h"
#include "utils/Utils.h"
#include "logging/Logger.h"
#include "utils/std/Result.h"
#include "wait-free-queues/utils/spin_lock.hpp"

#define FRAGMENT_MIN_SIZE 1024

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    ip::tcp::socket socket;
    logger_t logger;

    uint8_t messageLengthHeaderBuffer[sizeof(memDbDataLength_t)];
    uint8_t typePacketHeaderBuffer[1];

    jaime::utils::spin_lock flagReadInProgress;
    std::atomic_uint64_t readRequestCounter;

    std::function<ip::tcp::socket()> resetConnectionCallback;

public:
    Connection(ip::tcp::socket socket, logger_t logger);

    void onRequest(std::function<void(const std::vector<uint8_t>&)> onRequestCallbackParam);

    void setResetConnection(std::function<ip::tcp::socket()> resetCallback);

    std::string getAddress();

    void readAsync(uint64_t timeout);

    std::result<std::vector<uint8_t>> readSync(uint64_t ms);

    void writeAsync(std::vector<uint8_t>& toWrite);

    std::size_t writeSync(std::vector<uint8_t>& toWrite);

    bool isOpen();

    void close();

private:
    std::size_t writeSyncFragmented(std::vector<uint8_t>& bytes);

    std::result<std::vector<uint8_t>> readPacket(uint8_t packetType, uint64_t timeoutMs);
    std::result<std::vector<uint8_t>> readPacketContent(uint64_t timeoutMs);
    std::result<std::vector<uint8_t>> readFragmentedPacket(uint64_t timeoutMs);

    std::size_t fragmentPacketAndSend(std::vector<uint8_t>& packet, std::function<std::size_t(std::vector<uint8_t>&)> sender);

    std::vector<uint8_t> addNoFragmentationHeader(std::vector<uint8_t>& vec);
    std::vector<uint8_t> addContentLengthHeader(std::vector<uint8_t>& vec, memDbDataLength_t contentLength);

    memDbDataLength_t readPacketLength();

    bool isFragmentPacket(uint8_t packetTypeHeader);
    bool isLastFragmentPacket(uint8_t packetTypeHeader);

    void enableTcpNoDelay();
    void setTcpReceiveBufferSize(std::size_t size);
    void setTcpSendBufferSize(std::size_t size);

    boost::system::error_code readWithTimeout(boost::asio::mutable_buffers_1 buffer, uint64_t timeoutMs);

    void resetConnection();
};

using connection_t = std::shared_ptr<Connection>;