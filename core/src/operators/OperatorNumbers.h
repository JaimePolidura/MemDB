#pragma once

#include "shared.h"

class OperatorNumbers {
public:
    static const constexpr uint8_t SET = 1; //4
    static const constexpr uint8_t GET = 2; //8
    static const constexpr uint8_t DEL = 3; //12
    static const constexpr uint8_t INC = 18;
    static const constexpr uint8_t CONTAINS = 17;

    static const constexpr uint8_t HEALTH_CHECK = 4; //16
    static const constexpr uint8_t SYNC_OPLOG = 5; //20
    static const constexpr uint8_t MOVE_OPLOG = 6; //24
    static const constexpr uint8_t NEXT_SYNC_OPLOG_SEGMENT = 7; //28
    static const constexpr uint8_t FIX_OPLOG_SEGMENT = 8;

    static const constexpr uint8_t CAS = 9;
    static const constexpr uint8_t CAS_PREPARE = 10;
    static const constexpr uint8_t CAS_ACCEPT = 11;

    static const constexpr uint8_t GET_CLUSTER_CONFIG = 12;
    static const constexpr uint8_t JOIN_CLUSTER_ANNOUNCE = 13;
    static const constexpr uint8_t LEAVE_CLUSTER_ANNOUNCE = 14;
    static const constexpr uint8_t DO_LEAVE_CLUSTER = 15;
    static const constexpr uint8_t GET_NODE_DATA = 16;
};