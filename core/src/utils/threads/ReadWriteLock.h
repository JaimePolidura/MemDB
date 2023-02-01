#pragma once

#include <atomic>
#include <condition_variable>

class ReadWriteLock {
private:
    int64_t readersCount;
    std::mutex readCounterLock;
    std::mutex lock;

public:
    void lockRead() {
        this->readCounterLock.lock();

        if(++this->readersCount == 1)
            this->lock.lock();

        this->readCounterLock.unlock();
    }

    void unlockRead() {
        this->readCounterLock.lock();

        if(--this->readersCount == 0)
            this->lock.unlock();

        this->readCounterLock.unlock();
    }

    void lockWrite() {
        this->lock.lock();
    }

    void unlockWrite() {
        this->lock.unlock();
    }
};