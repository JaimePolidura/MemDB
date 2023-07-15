#pragma once

#include "messages/request/Request.h"
#include "utils/Utils.h"

#include "shared.h"

class RequestSerializer {
public:
    std::vector<uint8_t> serialize(const Request& request);
};