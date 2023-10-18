#pragma once

#include "shared.h"

class OperatorNumbers {
public:
    static const constexpr uint8_t SET = 0x01;
    static const constexpr uint8_t GET = 0x02;
    static const constexpr uint8_t DEL = 0x03;
    static const constexpr uint8_t HEALTH_CHECK = 0x04;
    static const constexpr uint8_t SYNC_OPLOG = 0x05;
    static const constexpr uint8_t MOVE_OPLOG = 0x06;
    static const constexpr uint8_t NEXT_SYNC_OPLOG_SEGMENT = 0x07;
};

