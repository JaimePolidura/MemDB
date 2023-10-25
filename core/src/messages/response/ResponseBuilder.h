#pragma once

#include "messages/response/Response.h"

#include "shared.h"

class ResponseBuilder {
private:
    std::vector<SimpleString<memDbDataLength_t>> _responseValue{};
    memdbRequestNumber_t _requestNumber{};
    uint64_t _timestamp{};
    uint8_t _errorCode{};
    bool _isSuccessful{};

public:
    ResponseBuilder();

    ResponseBuilder * timestamp(uint64_t timestamp);
    ResponseBuilder * error(uint8_t errorCode);
    ResponseBuilder * success();
    ResponseBuilder * requestNumber(memdbRequestNumber_t requestNumber);
    ResponseBuilder * value(const SimpleString<memDbDataLength_t>& value);
    ResponseBuilder * values(const std::vector<SimpleString<memDbDataLength_t>>& values);

    Response build();

    static ResponseBuilder builder();
};