#include "utils/threads/SharedLock.h"

void SharedLock::lockShared() {
    this->sharedCountLock.lock();

    if(++this->lockedSharedCount == 1)
        this->lock.lock();

    this->sharedCountLock.unlock();
}

void SharedLock::unlockShared() {
    this->sharedCountLock.lock();

    if(--this->lockedSharedCount == 0)
        this->lock.unlock();

    this->sharedCountLock.unlock();
}

void SharedLock::lockExclusive() {
    this->lock.lock();
    this->lockedExclusiveCount++;
}

void SharedLock::unlockExclusive() {
    if(--this->lockedExclusiveCount == 0){
        this->lockedExclusiveCount = 0;
        this->lock.unlock();
    }
}