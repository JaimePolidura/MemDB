#pragma once

#include "shared.h"
#include "Response.h"
#include "utils/Utils.h"

class ResponseDeserializer {
public:
    Response deserialize(const std::vector<uint8_t>& buffer) {
        auto requestNumber = Utils::parseFromBuffer<defaultMemDbRequestNumberLength_t>(buffer);
        auto timestamp = Utils::parseFromBuffer<uint64_t>(buffer, 4);
        auto success = ((uint8_t) (buffer[12] << 7)) != 0;
        auto errorCode = static_cast<uint8_t>(buffer[12] >> 1);
        auto lengthResponse = Utils::parseFromBuffer<uint32_t>(buffer, 13);
        auto responseBodyPtr = Utils::copyFromBuffer(buffer, 17, buffer.size() - 1);

        return Response{success, errorCode, timestamp, requestNumber, SimpleString(responseBodyPtr, lengthResponse)};
    }
};