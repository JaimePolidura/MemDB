#pragma once

#include "share.h"

#include "utils/strings/SimpleString.h"
#include "utils/clock/LamportClock.h"
#include "db/counters/Counter.h"

enum NodeType {
    DATA,
    COUNTER
};

template<typename SizeValue>
class DataAVLNode {
public:
    SimpleString<SizeValue> value;
    LamportClock timestamp{};

    DataAVLNode(SimpleString<SizeValue> value, LamportClock timestamp): value(value), timestamp(timestamp) {}
};

class CounterAVLNode {
public:
    Counter counter;
};