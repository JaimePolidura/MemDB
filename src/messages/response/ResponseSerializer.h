#pragma once

#include <stdint.h>
#include <vector>
#include <memory>

#include "Response.h"

class ResponseSerializer {
public:
    std::shared_ptr<std::vector<uint8_t>> serialize(std::shared_ptr<Response> response) {
        std::shared_ptr<std::vector<uint8_t>> serialized = std::make_shared<std::vector<uint8_t>>();

        serialized->push_back(response->errorCode << 1 | response->isSuccessful);
        serialized->push_back(response->lengthResponse);

        for (int i = 0; i < response->lengthResponse; ++i)
            serialized->push_back(* (response->response + i));

        return serialized;
    }
};