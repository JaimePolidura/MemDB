#pragma once

#include "shared.h"

#include "utils/strings/SimpleString.h"
#include "utils/clock/LamportClock.h"
#include "memdbtypes.h"

struct Response {
public:
    SimpleString<memDbDataLength_t> responseValue;
    memdbRequestNumber_t requestNumber;
    LamportClock timestamp{0, 0};
    uint8_t errorCode;
    bool isSuccessful;
    
    Response(bool isSuccessful, uint8_t errorCode, LamportClock timestamp, memdbRequestNumber_t reqNumber, const SimpleString<memDbDataLength_t> &response);

    Response() = default;

    memDbDataLength_t getTotalLength() const;

    SimpleString<memDbDataLength_t> getResponseValueAtOffset(uint32_t initOffset, std::size_t size) const;

    bool hasErrorCode(uint8_t errorCode);

    static Response success(const SimpleString<memDbDataLength_t> &response, LamportClock timestamp = {0, 0});

    static Response success(LamportClock timestamp = {});

    static Response error(uint8_t errorCode);

    static Response error(uint8_t errorCode, memdbRequestNumber_t requestNumber, LamportClock timestamp = {0, 0});
};