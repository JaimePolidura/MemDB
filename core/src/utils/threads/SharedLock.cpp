#include "utils/threads/SharedLock.h"

void SharedLock::lockShared() {
    this->sharedCounterLock.lock();

    if(++this->sharedCount == 1)
        this->lock.lock();

    this->sharedCounterLock.unlock();
}

void SharedLock::unlockShared() {
    this->sharedCounterLock.lock();

    if(--this->sharedCount == 0)
        this->lock.unlock();

    this->sharedCounterLock.unlock();
}

void SharedLock::lockExclusive() {
    this->lock.lock();
}

void SharedLock::unlockExclusive() {
    this->lock.unlock();
}