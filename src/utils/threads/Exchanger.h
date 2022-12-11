#pragma once

#include <condition_variable>
#include <mutex>
#include <memory>

template<typename T>
class Exchanger {
private:
    std::shared_ptr<T> item;
    std::condition_variable itemDequeued;
    std::condition_variable itemQueued;
    std::mutex lock;
    bool hasItem;

public:
    void asyncEnqueue(std::shared_ptr<T> itemToExchange) {
        std::unique_lock uniqueLock(this->lock);

        this->item = itemToExchange;
        this->hasItem = true;

        this->itemQueued.notify_all();

        this->lock.unlock();
    }

    void enqueue(std::shared_ptr<T> itemToExchange) {
        std::unique_lock uniqueLock(this->lock);

        this->item = itemToExchange;
        this->hasItem = true;

        this->itemQueued.notify_all();

        this->itemDequeued.wait(uniqueLock, [this]{ return !this->hasItem;});

        this->lock.unlock();
    }

    std::shared_ptr<T> dequeue() {
        std::unique_lock uniqueLock(this->lock);

        this->itemQueued.wait(uniqueLock, [this]{ return this->hasItem;});

        std::shared_ptr<T> itemToDequeue = this->item;

        this->hasItem = false;

        this->itemDequeued.notify_all();

        this->lock.unlock();

        return itemToDequeue;
    }

    bool isEmpty() {
        return !this->hasItem;
    }
};