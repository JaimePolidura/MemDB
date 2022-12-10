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

public:
    Exchanger(): item(nullptr) {}

    void enqueue(std::shared_ptr<T> itemToExchange) {
        std::unique_lock uniqueLock(this->lock);

        this->item = itemToExchange;

        this->itemQueued.notify_all();

        this->itemDequeued.wait(uniqueLock, [this]{ return this->item.get() == nullptr;});

        this->lock.unlock();
    }

    std::shared_ptr<T> dequeue() {
        std::unique_lock uniqueLock(this->lock);

        this->itemQueued.wait(uniqueLock, [this]{ return this->item.get() != nullptr;});

        std::shared_ptr<T> itemToDequeue = this->item;

        this->item = nullptr;

        this->itemDequeued.notify_all();

        this->lock.unlock();

        return itemToDequeue;
    }

    bool isEmpty() {
        return this->item == nullptr;
    }
};