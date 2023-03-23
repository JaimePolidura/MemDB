#pragma once

#include "shared.h"
#include "messages/response/Response.h"
#include "utils/Utils.h"

class ResponseSerializer {
public:
    std::vector<uint8_t> serialize(const Response& response) {
        std::vector<uint8_t> serialized{};

        Utils::appendToBuffer(response.requestNumber, serialized);
        Utils::appendToBuffer(response.timestamp, serialized);
        serialized.push_back(response.errorCode << 1 | response.isSuccessful);
        Utils::appendToBuffer(response.responseValue.size, serialized);
        Utils::appendToBuffer(response.responseValue.data(), response.responseValue.size, serialized);

        return serialized;
    }
};