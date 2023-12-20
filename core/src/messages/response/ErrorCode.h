#pragma once

#include "shared.h"

class ErrorCode {
public:
    static const constexpr uint8_t AUTH_ERROR = 0;
    static const constexpr uint8_t UNKNOWN_KEY = 1;
    static const constexpr uint8_t UNKNOWN_OPERATOR = 2;
    static const constexpr uint8_t ALREADY_REPLICATED = 3;
    static const constexpr uint8_t NOT_AUTHORIZED = 4;
    static const constexpr uint8_t INVALID_NODE_STATE = 5;
    static const constexpr uint8_t INVALID_PARTITION = 6;
    static const constexpr uint8_t INVALID_CALL = 7;
    static const constexpr uint8_t SYNC_OP_LOG_EOF = 8;
    static const constexpr uint8_t UNFIXABLE_CORRUPTED_OPLOG_SEGMENT = 9;
    static const constexpr uint8_t CAS_FAILED = 10;
    static const constexpr uint8_t NODE_NOT_FOUND = 11;
    static const constexpr uint8_t INVALID_TYPE = 12;
};
