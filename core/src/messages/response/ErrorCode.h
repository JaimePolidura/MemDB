#pragma once

#include "shared.h"

class ErrorCode {
public:
    static const constexpr uint8_t AUTH_ERROR = 0x00;
    static const constexpr uint8_t UNKNOWN_KEY = 0x01;
    static const constexpr uint8_t UNKNOWN_OPERATOR = 0x02;
    static const constexpr uint8_t ALREADY_REPLICATED = 0x03;
    static const constexpr uint8_t NOT_AUTHORIZED = 0x04;
    static const constexpr uint8_t INVALID_NODE_STATE = 0x05;
    static const constexpr uint8_t INVALID_PARTITION = 0x06;
    static const constexpr uint8_t INVALID_CALL = 0x07;
};
