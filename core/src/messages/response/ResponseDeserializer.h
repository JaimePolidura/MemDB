#pragma once

#include "shared.h"
#include "messages/response/Response.h"
#include "utils/Utils.h"

class ResponseDeserializer {
public:
    //We assume that the buffer doest contain the total reponse length, which occupies 4 bytes
    Response deserialize(const std::vector<uint8_t>& buffer);
};