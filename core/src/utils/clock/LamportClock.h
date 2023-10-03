#pragma once

#include "shared.h"

class LamportClock {
public:
    std::atomic_uint64_t counter;
    uint16_t nodeId;

    explicit LamportClock(uint16_t nodeId);

    LamportClock(uint16_t nodeId, uint64_t counter);

    LamportClock& operator=(const LamportClock& other);

    virtual uint64_t tick(uint64_t other);

    //Returns if this clock is bigger than the passed by arguments
    bool compare(uint64_t otherCount, uint16_t otherNodeId);

    bool operator<(const LamportClock& other); //this <= other

    bool operator<=(const LamportClock& other); //this <= other

    bool operator>(const LamportClock& other); // this > other

    bool operator>=(const LamportClock& other); // this >= other

    bool operator==(const LamportClock& other); // this == other
};

using lamportClock_t = std::shared_ptr<LamportClock>;