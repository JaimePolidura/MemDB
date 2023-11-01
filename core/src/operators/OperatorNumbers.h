#pragma once

#include "shared.h"

class OperatorNumbers {
public:
    static const constexpr uint8_t SET = 0x01; //4
    static const constexpr uint8_t GET = 0x02; //8
    static const constexpr uint8_t DEL = 0x03; //12
    static const constexpr uint8_t HEALTH_CHECK = 0x04; //16
    static const constexpr uint8_t SYNC_OPLOG = 0x05; //20
    static const constexpr uint8_t MOVE_OPLOG = 0x06; //24
    static const constexpr uint8_t NEXT_SYNC_OPLOG_SEGMENT = 0x07; //28
    static const constexpr uint8_t FIX_OPLOG_SEGMENT = 0x08;

    static const constexpr uint8_t CAS = 0x09;
    static const constexpr uint8_t CAS_PREPARE = 0x10;
    static const constexpr uint8_t CAS_ACCEPT = 0x11;
};

