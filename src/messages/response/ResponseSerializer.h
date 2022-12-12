#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "Response.h"

class ResponseSerializer {
    std::shared_ptr<std::vector<uint8_t>> serialize(const Response& response) {
        std::shared_ptr<std::vector<uint8_t>> serialized = std::make_shared<std::vector<uint8_t>>();

        serialized->data()[0] = response.errorType << 1 | response.isSuccessful;
        serialized->data()[1] = response.lengthResponse;

        for (int i = 0; i < response.lengthResponse; ++i)
            serialized->data()[i + 2] = * (response.response + i);

        return serialized;
    }
};