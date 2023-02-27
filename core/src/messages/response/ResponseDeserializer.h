#pragma once

#include <vector>
#include "Response.h"

#include "utils/Utils.h"

class ResponseDeserializer {
public:
    Response deserialize(const std::vector<uint8_t>& buffer) {
        auto responseNumber = Utils::parseFromBuffer<uint64_t>(buffer);
        auto timestamp = Utils::parseFromBuffer<uint64_t>(buffer, 8);
        auto success = (bool) (buffer[16] << 7);
        auto errorCode = static_cast<uint8_t>(buffer[16] >> 1);
        auto lengthResponse = Utils::parseFromBuffer<uint32_t>(buffer, 17);
        auto responseBodyPtr = Utils::copyFromBuffer<uint8_t>(buffer, 18, buffer.size() - 1);

        return Response{success, errorCode, timestamp, SimpleString(responseBodyPtr, lengthResponse)};
    }
};