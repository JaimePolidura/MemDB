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

    static const constexpr uint8_t GET_CLUSTER_CONFIG = 0x12;
    static const constexpr uint8_t JOIN_CLUSTER_ANNOUNCE = 0x13;
    static const constexpr uint8_t LEAVE_CLUSTER_ANNOUNCE = 0x14;
    static const constexpr uint8_t DO_LEAVE_CLUSTER = 0x15;
    static const constexpr uint8_t GET_NODE_DATA = 0x16;
};