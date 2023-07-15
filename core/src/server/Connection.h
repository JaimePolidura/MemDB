#pragma once

#include "shared.h"
#include "memdbtypes.h"
#include "utils/Utils.h"

using namespace boost::asio;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    uint8_t messageLengthBuffer[sizeof(memDbDataLength_t)];
    std::function<void(const std::vector<uint8_t>&)> onRequestCallback;
    ip::tcp::socket socket;

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
    std::vector<uint8_t> readSocketBufferLenght(memDbDataLength_t length);
};

using connection_t = std::shared_ptr<Connection>;