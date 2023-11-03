#include "ExponentialRandomizedTimer.h"

ExponentialRandomizedTimer::ExponentialRandomizedTimer(uint64_t startMs, uint64_t maxTimeoutMs): startMs(startMs), maxTimeoutMs(maxTimeoutMs), nextSleepingMs(startMs) {}

void ExponentialRandomizedTimer::sleep() {
    std::this_thread::sleep_for(std::chrono::milliseconds(randomize(this->nextSleepingMs)));
    this->nextSleepingMs = std::max(this->nextSleepingMs * this->nextSleepingMs, this->maxTimeoutMs);
}

void ExponentialRandomizedTimer::reset() {
    this->nextSleepingMs = this->startMs;
}

uint64_t ExponentialRandomizedTimer::randomize(uint64_t timeMs) {
    return timeMs * distribution(generator);
}