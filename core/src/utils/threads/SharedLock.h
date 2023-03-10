#pragma once

#include "shared.h"

class SharedLock {
private:
    int64_t sharedCount;
    std::mutex sharedCounterLock;
    std::mutex lock;

public:
    void lockShared() {
        this->sharedCounterLock.lock();

        if(++this->sharedCount == 1)
            this->lock.lock();

        this->sharedCounterLock.unlock();
    }

    void unlockShared() {
        this->sharedCounterLock.lock();

        if(--this->sharedCount == 0)
            this->lock.unlock();

        this->sharedCounterLock.unlock();
    }

    void lockExclusive() {
        this->lock.lock();
    }

    void unlockExclusive() {
        this->lock.unlock();
    }
};