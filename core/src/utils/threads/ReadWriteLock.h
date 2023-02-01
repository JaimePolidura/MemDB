#pragma once

#include <atomic>
#include <condition_variable>

class ReadWriteLock {
private:
    std::atomic_int64_t readersCount; //Readers in crital section
    std::atomic_int64_t writersCount; //Writer in critial section + nº writers waiting
    std::mutex writerLock;

    std::condition_variable noWriterCondition;
    std::mutex noWriterConditionLock;

    std::condition_variable noReadersCondition;
    std::mutex noReadersConditionLock;

public:
    void lockRead() {
        std::unique_lock noWriterConditionUniqueLock(this->noWriterConditionLock);
        this->noWriterCondition.wait(noWriterConditionUniqueLock, [this]{return this->writersCount.load() == 0;});

        this->readersCount.fetch_add(1);
    }

    void unlockRead() {
        this->readersCount.fetch_sub(1);

        if(this->readersCount.load() == 0)
            this->noReadersCondition.notify_one();
    }

    void lockWrite() {
        std::unique_lock noReadersConditionUniqueLock(this->noReadersConditionLock);

        this->writersCount.fetch_add(1);
        this->noReadersCondition.wait(noReadersConditionUniqueLock, [this]{return this->readersCount.load() == 0;});

        this->writerLock.lock();
    }

    void unlockWrite() {
        this->writersCount.fetch_sub(1);
        if(this->writersCount.load() == 0)
            this->noWriterCondition.notify_all();

        this->writerLock.unlock();
    }
};