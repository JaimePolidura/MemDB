#include "utils/clock/LamportClock.h"

LamportClock::LamportClock(uint16_t nodeId): nodeId(nodeId), counter(0) {}

LamportClock::LamportClock(uint16_t nodeId, uint64_t counter): nodeId(nodeId), counter(counter) {}

LamportClock& LamportClock::operator=(const LamportClock& other) {
    this->nodeId = other.nodeId;
    this->counter.store(other.counter.load(std::memory_order_acquire), std::memory_order_release);

    return * this;
}

uint64_t LamportClock::tick(uint64_t other) {
    uint64_t actualCounterValue = this->counter.load(std::memory_order_acquire);
    uint64_t max = std::max(other, actualCounterValue);
    uint64_t newCounter = max + 1;

    do {
        actualCounterValue = this->counter.load();
        newCounter = std::max(actualCounterValue, other) + 1;
    } while(!this->counter.compare_exchange_weak(actualCounterValue, newCounter, std::memory_order_release));

    return newCounter;
}

//Returns if this clock is bigger than the passed by arguments
bool LamportClock::compare(uint64_t otherCount, uint16_t otherNodeId) {
    uint64_t selfCounterValue = this->counter.load(std::memory_order_acquire);

    return selfCounterValue > otherCount ||
           (selfCounterValue == otherCount && this->nodeId > otherNodeId);
}

bool LamportClock::operator<(const LamportClock& other) { //this <= other
    uint64_t selfCounterValue = this->counter.load(std::memory_order_acquire);
    uint64_t otherCounterValue = other.counter.load(std::memory_order_acquire);

    return selfCounterValue < otherCounterValue ||
           (selfCounterValue == otherCounterValue && this->nodeId < other.nodeId);
}

bool LamportClock::operator<=(const LamportClock& other) { //this <= other
    uint64_t selfCounterValue = this->counter.load(std::memory_order_acquire);
    uint64_t otherCounterValue = other.counter.load(std::memory_order_acquire);

    return (otherCounterValue == selfCounterValue && other.nodeId == this->nodeId) &&
            selfCounterValue < otherCounterValue ||
           (selfCounterValue == otherCounterValue && this->nodeId < other.nodeId);
}

bool LamportClock::operator>(const LamportClock& other){ // this > other
    uint64_t selfCounterValue = this->counter.load(std::memory_order_acquire);
    uint64_t otherCounterValue = other.counter.load(std::memory_order_acquire);

    return selfCounterValue > otherCounterValue ||
           (selfCounterValue == otherCounterValue && this->nodeId > other.nodeId);
}

bool LamportClock::operator>=(const LamportClock& other){ // this >= other
    uint64_t selfCounterValue = this->counter.load(std::memory_order_acquire);
    uint64_t otherCounterValue = other.counter.load(std::memory_order_acquire);

    return (otherCounterValue == selfCounterValue && other.nodeId == this->nodeId) &&
           selfCounterValue > otherCounterValue ||
           (selfCounterValue == otherCounterValue && this->nodeId > other.nodeId);
}

bool LamportClock::operator==(const LamportClock& other){ // this == other
    uint64_t selfCounterValue = this->counter.load(std::memory_order_acquire);
    uint64_t otherCounterValue = other.counter.load(std::memory_order_acquire);

    return selfCounterValue == otherCounterValue && other.nodeId == this->nodeId;
}