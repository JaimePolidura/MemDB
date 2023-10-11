#include "messages/response/ResponseDeserializer.h"

Response ResponseDeserializer::deserialize(const std::vector<uint8_t>& buffer) {
    auto requestNumber = Utils::parseFromBuffer<memdbRequestNumber_t>(buffer);
    auto timestamp = Utils::parseFromBuffer<uint64_t>(buffer, sizeof(memdbRequestNumber_t));
    auto success = ((uint8_t) (buffer[8 + sizeof(memdbRequestNumber_t)] << 7)) != 0;
    auto errorCode = static_cast<uint8_t>(buffer[8 + sizeof(memdbRequestNumber_t)] >> 1);
    auto lengthResponse = Utils::parseFromBuffer<uint32_t>(buffer, 8 + sizeof(memdbRequestNumber_t) + 1);
    auto responseBodyPtr = Utils::copyFromBuffer(buffer, 8 + sizeof(memdbRequestNumber_t) + 1 + 4, buffer.size() - 1);

    return Response{success, errorCode, timestamp, requestNumber, SimpleString(responseBodyPtr, lengthResponse)};
}