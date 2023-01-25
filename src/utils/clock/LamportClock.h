#pragma once

#include <algorithm>
#include <cstdint>
#include <atomic>

class LamportClock {
private:
    std::atomic_uint64_t counter;
    uint16_t nodeId;

public:
    LamportClock(uint16_t nodeId): nodeId(nodeId), counter(0) {}

    uint64_t tick(uint64_t other) {
        uint64_t actualCountervalue = this->counter.load();
        uint64_t max = std::max(other, actualCountervalue);
        uint64_t newCounter = max + 1;

        do {
            actualCountervalue = this->counter.load();
            newCounter = std::max(actualCountervalue, other) + 1;
        } while(this->counter.compare_exchange_strong(actualCountervalue, newCounter));

        return newCounter;
    }

    bool operator<(const LamportClock& other) { //this <= other
        return other.counter.load() > this->counter.load() || other.nodeId > this->nodeId;
    }

    bool operator<=(const LamportClock& other) { //this <= other
        return other.counter.load() >= this->counter.load() || other.nodeId >= this->nodeId;
    }

    bool operator>(const LamportClock& other){ // this > other
        return other.counter.load() < this->counter.load() || other.nodeId < this->nodeId;
    }

    bool operator>=(const LamportClock& other){ // this >= other
        return other.counter.load() <= this->counter.load() || other.nodeId <= this->nodeId;
    }

    bool operator==(const LamportClock& other){ // this == other
        return other.counter.load() == this->counter.load() && other.nodeId == this->nodeId;
    }
};