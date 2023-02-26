#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "Response.h"

#include "utils/Utils.h"

class ResponseSerializer {
public:
    std::vector<uint8_t> serialize(const Response& response) {
        std::vector<uint8_t> serialized{};

        Utils::parseToBuffer(response.requestNumber, serialized);
        Utils::parseToBuffer(response.timestamp, serialized, 8);
        serialized.push_back(response.errorCode << 1 | response.isSuccessful);

        Utils::parseToBuffer(response.responseValue.getSizeOfStringLengthType(), serialized, 17);
        Utils::appendToBuffer(response.responseValue.data(), response.responseValue.size, serialized);

        return serialized;
    }
};