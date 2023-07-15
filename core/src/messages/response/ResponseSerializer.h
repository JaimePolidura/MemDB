#pragma once

#include "shared.h"
#include "messages/response/Response.h"
#include "utils/Utils.h"

class ResponseSerializer {
public:
    std::vector<uint8_t> serialize(const Response& response);
};