#pragma once

#include <condition_variable>
#include <mutex>
#include <memory>
#include <atomic>

template<typename T>
class Node {
public:
    std::shared_ptr<Node<T>> next;
    std::shared_ptr<Node<T>> prev;
    T data;

public:
    Node(const T& dataCons, std::shared_ptr<Node<T>> nextCons, std::shared_ptr<Node<T>> prevCons): data(dataCons), next(nextCons), prev(prevCons) {}

    Node(T&& dataCons, std::shared_ptr<Node<T>> nextCons, std::shared_ptr<Node<T>> prevCons): data(dataCons), next(nextCons), prev(prevCons) {}
};

template<typename T>
class BlockingQueue {
private:
    std::shared_ptr<Node<T>> head; //Pointing to element to dequeue
    std::shared_ptr<Node<T>> tail; //Pointing to last element added
    std::condition_variable notEmtpyCondition;
    std::mutex lock;
    bool stop;
    int size;

public:
    void enqueue(const T& data) {
        T copyOfData = data;

        this->lock.lock();

        std::shared_ptr<Node<T>> newNodeToEnqueue = std::make_shared<Node<T>>(copyOfData, nullptr, nullptr);

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
            return this->head->data;

        std::shared_ptr<Node<T>> nodeToDequeue = this->head;
        this->head = this->head->prev;

        this->size--;

        uniqueLock.unlock();

        return nodeToDequeue->data;
    }

    int getSize() {
        return this->size;
    }
};