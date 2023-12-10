#pragma once

#include "messages/response/Response.h"

#include "shared.h"

class ResponseBuilder {
private:
    std::vector<SimpleString<memDbDataLength_t>> _responseValue{};
    memdbRequestNumber_t _requestNumber{};
    LamportClock _timestamp{0, 0};
    uint8_t _errorCode{};
    bool _isSuccessful{};

public:
    ResponseBuilder();

    ResponseBuilder * timestampCounter(uint64_t timestamp);
    ResponseBuilder * timestamp(LamportClock timestamp);
    ResponseBuilder * error(uint8_t errorCode);
    ResponseBuilder * success();
    ResponseBuilder * requestNumber(memdbRequestNumber_t requestNumber);
    ResponseBuilder * value(const SimpleString<memDbDataLength_t>& value);
    ResponseBuilder * valueIfSuccessful(bool isSuccess, const SimpleString<memDbDataLength_t>& value);
    ResponseBuilder * values(const std::vector<SimpleString<memDbDataLength_t>>& values);
    ResponseBuilder * isSuccessful(bool isSuccessful, uint8_t errorCodeIfUnsuccessful);

    Response build();

    static ResponseBuilder builder();

private:
    SimpleString<memDbDataLength_t> buildResponseValue();
};