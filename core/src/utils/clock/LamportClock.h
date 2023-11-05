#pragma once

#include "shared.h"

class LamportClock {
public:
    enum UpdateClockStrategy {
        NONE,
        SET_MAX,
        TICK,
    };

    std::atomic_uint64_t counter;
    uint16_t nodeId;

    explicit LamportClock(uint16_t nodeId);

    LamportClock(uint16_t nodeId, uint64_t counter);

    LamportClock(const LamportClock& other);

    LamportClock() = default;

    LamportClock& operator=(const LamportClock& other);

    virtual uint64_t tick(uint64_t other);

    uint64_t update(LamportClock::UpdateClockStrategy updateStrategy, uint64_t otherCounter);

    uint64_t setMax(uint64_t other);

    uint64_t getCounterValue();

    std::string toString();

    //Returns if this clock is bigger than the passed by arguments
    bool compare(uint64_t otherCount, uint16_t otherNodeId);

    bool operator<(const LamportClock& other); //this <= other

    bool operator<=(const LamportClock& other); //this <= other

    bool operator>(const LamportClock& other); // this > other

    bool operator>=(const LamportClock& other); // this >= other

    bool operator==(const LamportClock& other); // this == other
};

using lamportClock_t = std::shared_ptr<LamportClock>;