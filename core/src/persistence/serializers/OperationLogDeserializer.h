#pragma once

#include "shared.h"

#include "messages/request/RequestDeserializer.h"

class OperationLogDeserializer {
private:
    RequestDeserializer requestDeserializer;

public:
    std::vector<OperationBody> deserializeAll(const std::vector<uint8_t>& bytes);
};