#pragma once

#include <cstdint>

class ErrorCode {
public:
    static const constexpr uint8_t AUTH_ERROR = 0x00;
    static const constexpr uint8_t UNKNOWN_KEY = 0x01;
    static const constexpr uint8_t UNKNOWN_OPERATOR = 0x02;
    static const constexpr uint8_t ALREADY_REPLICATED = 0x03;
};
