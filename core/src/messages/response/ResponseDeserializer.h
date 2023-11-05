#pragma once

#include "shared.h"
#include "messages/response/Response.h"
#include "utils/Utils.h"

class ResponseDeserializer {
public:
    std::result<Response> deserialize(const std::vector<uint8_t>& buffer);
};