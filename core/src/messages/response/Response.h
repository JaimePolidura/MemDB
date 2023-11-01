#pragma once

#include "shared.h"

#include "utils/strings/SimpleString.h"
#include "memdbtypes.h"

struct Response {
public:
    SimpleString<memDbDataLength_t> responseValue;
    memdbRequestNumber_t requestNumber;
    uint64_t timestamp;
    uint8_t errorCode;
    bool isSuccessful;
    
    Response(bool isSuccessful, uint8_t errorCode, uint64_t timestamp, memdbRequestNumber_t reqNumber, const SimpleString<memDbDataLength_t> &response);

    Response() = default;

    memDbDataLength_t getTotalLength() const;

    SimpleString<memDbDataLength_t> getResponseValueAtOffset(uint32_t initOffset, std::size_t size) const;

    bool hasErrorCode(uint8_t errorCode);

    static Response success(const SimpleString<memDbDataLength_t> &response, uint64_t timestamp = 0);

    static Response success(uint64_t timestamp = 0);

    static Response error(uint8_t errorCode);

    static Response error(uint8_t errorCode, memdbRequestNumber_t requestNumber, uint64_t timestamp = 0);
};