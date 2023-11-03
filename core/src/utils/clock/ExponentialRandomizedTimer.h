#pragma once

#include "shared.h"

class ExponentialRandomizedTimer {
public:
    ExponentialRandomizedTimer(uint64_t startMs, uint64_t maxTimeoutMs);

    void sleep();

    void reset();

private:
    uint64_t startMs;
    uint64_t maxTimeoutMs;
    uint64_t nextSleepingMs;

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution{0.75, 1.25};

    uint64_t randomize(uint64_t timeMs);
};
