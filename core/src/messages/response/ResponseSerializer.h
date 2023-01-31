#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "Response.h"

#include "utils/Utils.h"

#define MAX_RESPONSE_SIZE 257

class ResponseSerializer {
public:
    std::vector<uint8_t> serialize(const Response& response) {
        std::vector<uint8_t> serialized{};
        serialized.reserve(MAX_RESPONSE_SIZE);

        Utils::parseToBuffer(response.requestNumber, serialized);
        Utils::parseToBuffer(response.timestamp, serialized, 8);

        serialized.push_back(response.errorCode << 1 | response.isSuccessful);
        serialized.push_back(response.responseValue.size);

        for (int i = 0; i < response.responseValue.size; ++i)
            serialized.push_back(* (response.responseValue + i));

        size_t writtenSize = serialized.size();
        for(int i = 0; i < MAX_RESPONSE_SIZE - writtenSize; i++)
            serialized.push_back(0x00);

        return serialized;
    }
};