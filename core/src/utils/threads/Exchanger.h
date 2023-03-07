#pragma once

#include <condition_variable>
#include <mutex>
#include <memory>

template<typename T>
class Exchanger {
private:
    T item;
    std::condition_variable itemDequeued;
    std::condition_variable itemQueued;
    std::mutex lock;
    bool hasItem;

public:
    void asyncEnqueue(const T& itemToExchange) {
        std::unique_lock uniqueLock(this->lock);

        this->item = std::move(itemToExchange);
        this->hasItem = true;

        this->itemQueued.notify_all();

        this->lock.unlock();
    }

    void enqueue(const T& itemToExchange) {
        std::unique_lock uniqueLock(this->lock);

        this->item = std::move(itemToExchange);
        this->hasItem = true;

        this->itemQueued.notify_all();

        this->itemDequeued.wait(uniqueLock, [this]{ return !this->hasItem;});

        this->lock.unlock();
    }

    T dequeue() {
        std::unique_lock uniqueLock(this->lock);

        this->itemQueued.wait(uniqueLock, [this]{ return this->hasItem;});

        T itemToDequeue = std::move(this->item);

        this->hasItem = false;

        this->itemDequeued.notify_all();

        this->lock.unlock();

        return itemToDequeue;
    }

    bool isEmpty() {
        return !this->hasItem;
    }
};