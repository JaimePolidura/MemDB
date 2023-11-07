#include "messages/response/ResponseSerializer.h"

std::vector<uint8_t> ResponseSerializer::serialize(const Response& response) {
    std::vector<uint8_t> serialized{};

    Utils::appendToBuffer(response.requestNumber, serialized);
    Utils::appendToBuffer(response.timestamp.counter, serialized);
    Utils::appendToBuffer(response.timestamp.nodeId, serialized);
    serialized.push_back(response.errorCode << 1 | response.isSuccessful);
    Utils::appendToBuffer(response.responseValue.size, serialized);
    Utils::appendToBuffer(response.responseValue.data(), response.responseValue.size, serialized);

    return serialized;
}