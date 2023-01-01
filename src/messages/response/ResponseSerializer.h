#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "Response.h"

#define MAX_RESPONSE_SIZE 257

class ResponseSerializer {
public:
    std::shared_ptr<std::vector<uint8_t>> serialize(const Response& response) {
        std::shared_ptr<std::vector<uint8_t>> serialized = std::make_shared<std::vector<uint8_t>>();
        serialized->reserve(MAX_RESPONSE_SIZE);

        for (std::size_t i = 0; i < sizeof(uint64_t); ++i)
            serialized->push_back(static_cast<uint8_t>(response.requestNumber >> (sizeof(uint64_t) * i)));

        serialized->push_back(response.errorCode << 1 | response.isSuccessful);
        serialized->push_back(response.lengthResponse);

        for (int i = 0; i < response.lengthResponse; ++i)
            serialized->push_back(* (response.response + i));

        size_t writtenSize = serialized->size();
        for(int i = 0; i < MAX_RESPONSE_SIZE - writtenSize; i++)
            serialized->push_back(0x00);

        return serialized;
    }
};