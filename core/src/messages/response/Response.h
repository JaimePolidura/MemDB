#pragma once

#include "shared.h"

#include "utils/strings/SimpleString.h"
#include "memdbtypes.h"

struct Response {
public:
    SimpleString<memDbDataLength_t> responseValue;
    memdbRequestNumberLength_t requestNumber;
    uint64_t timestamp;
    uint8_t errorCode;
    bool isSuccessful;

    Response(bool isSuccessful, uint8_t errorCode, uint64_t timestamp, memdbRequestNumberLength_t reqNumber, const SimpleString<memDbDataLength_t> &response);
    
    memDbDataLength_t getTotalLength() const;

    static Response success(const SimpleString<memDbDataLength_t> &response, uint64_t timestamp = 0);

    static Response success(uint64_t timestamp = 0);

    static Response error(uint8_t errorCode);

    static Response error(uint8_t errorCode, memdbRequestNumberLength_t requestNumber, uint64_t timestamp = 0);
};