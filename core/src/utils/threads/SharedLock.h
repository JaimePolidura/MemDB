#pragma once

#include "shared.h"

class SharedLock {
private:
    int64_t sharedCount;
    std::mutex sharedCounterLock;
    std::mutex lock;

public:
    void lockShared();

    void unlockShared();

    void lockExclusive();

    void unlockExclusive();
};