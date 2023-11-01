#pragma once

#include "shared.h"

class CounterLatch {
private:
    std::atomic_int counter{0};
    std::condition_variable cond;
    std::mutex lock;

public:
    /**
     * Returns if operation was successful, not timed out
     */
    bool awaitMinOrEq(int minValue, uint64_t timeoutMs);

    void increase();
};
