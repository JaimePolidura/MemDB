#pragma once

#include <condition_variable>
#include <mutex>
#include <memory>
#include <atomic>

template<typename T>
class BlockingQueueNode {
public:
    std::shared_ptr<BlockingQueueNode<T>> next;
    std::shared_ptr<BlockingQueueNode<T>> prev;
    T value;

public:
    BlockingQueueNode(const T& dataCons, std::shared_ptr<BlockingQueueNode<T>> nextCons, std::shared_ptr<BlockingQueueNode<T>> prevCons):
        value(dataCons), next(nextCons), prev(prevCons) {}

    BlockingQueueNode(T&& dataCons, std::shared_ptr<BlockingQueueNode<T>> nextCons, std::shared_ptr<BlockingQueueNode<T>> prevCons):
        value(dataCons), next(nextCons), prev(prevCons) {}
};

template<typename T>
class BlockingQueue {
private:
    std::shared_ptr<BlockingQueueNode<T>> head; //Pointing to element to dequeue
    std::shared_ptr<BlockingQueueNode<T>> tail; //Pointing to last element added
    std::condition_variable notEmtpyCondition;
    std::mutex lock;
    bool stop;
    int size;

public:
    void enqueue(const T& data) {
        T copyOfData = data;

        this->lock.lock();

        std::shared_ptr<BlockingQueueNode<T>> newNodeToEnqueue = std::make_shared<BlockingQueueNode<T>>(copyOfData, nullptr, nullptr);

        if(this->size == 0){
            this->head = this->tail = newNodeToEnqueue;
        }else if(this->size == 1){
            this->head->prev = newNodeToEnqueue;
            newNodeToEnqueue->next = this->head;
            this->tail = newNodeToEnqueue;
        }else{
            this->tail->prev = newNodeToEnqueue;
            newNodeToEnqueue->next = this->tail;
            this->tail = newNodeToEnqueue;
        }

        this->size++;

        this->lock.unlock();

        this->notEmtpyCondition.notify_all();
    }

    void stopNow() {
        this->lock.lock();

        this->stop = true;
        this->notEmtpyCondition.notify_all();

        this->lock.unlock();
    }

    T& dequeue() {
        std::unique_lock uniqueLock(this->lock);
        this->notEmtpyCondition.wait(uniqueLock, [this]{ return this->size > 0 || stop; });

        if(this->stop)
            return this->head->value;

        std::shared_ptr<BlockingQueueNode<T>> nodeToDequeue = this->head;
        this->head = this->head->prev;

        this->size--;

        uniqueLock.unlock();

        return nodeToDequeue->value;
    }

    int getSize() {
        return this->size;
    }
};