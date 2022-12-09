#pragma once

#include <condition_variable>
#include <atomic>
#include <mutex>
#include <memory>

template<typename T>
class Node {
    std::shared_ptr<std::atomic_ref<Node<T>>> next;
    T& data;

    Node() = default;

    Node(const T& dataCons, std::atomic_ref<Node<T> *> nextCons): data(data), next(nextCons) {}
};

template<typename T>
class LockFreeQueue {
private:
    std::atomic_ref<Node<T>> head; //Pointing to element to dequeue
    std::atomic_ref<std::shared_ptr<Node<T>>> tail; //Pointing to last element added
    std::atomic_int size;
    std::mutex m_mutex;
    std::condition_variable m_cond;

public:
    LockFreeQueue(): head(), tail() {
        Node<T> sentinelHeadNode = std::make_shared<Node<T>>();
        this->head = sentinelHeadNode;
    }

    void enqueue(const T& data) {
        if(this->size == 0){
            this->m_mutex.lock();
            this->m_mutex.unlock();
        }

        std::shared_ptr<Node<T>> nodeToInsert = std::make_shared<Node<T>>(data, this->tail);

        while (true) {
            std::shared_ptr<Node<T>> last = this->tail->load();
            std::shared_ptr<Node<T>> nextToLast = last->next.load();

            if(last == nextToLast){
                if(nextToLast == nullptr){
                    if(last->next.compare_exchange_strong(nullptr, nodeToInsert)){
                        this->tail->compare_exchange_strong(last, nodeToInsert);
                        this->size++;
                        this->m_cond.notify_all();
                        return;
                    }
                }else{
                    this->tail->compare_exchange_strong(last, nextToLast);
                }
            }
        }
    }

    const T& dequeue() {
        if(this->isEmpty())
            this->waitUntilNotEmtpy();

        while (true) {
            std::shared_ptr<Node<T>> first = this->head->load();
            std::shared_ptr<Node<T>> last = this->tail->load();
            std::shared_ptr<Node<T>> next = first.next->load();

            if(first == this->head->get()){
                if(first == this->tail.get()){
                    this->tail.compare_exchange_strong(last, next);
                    continue;
                }

                if(this->head->compare_exchange_strong(first, next)){
                    this->size--;
                    const T& dataFirstNode = first;

                    return dataFirstNode;
                }
            }
        }
    }

private:
    void waitUntilNotEmtpy() {
        while (this->isEmpty()){
            this->m_mutex.lock();
            m_cond.wait(this->m_mutex);
        }

        this->m_mutex.unlock();
    }

    bool isEmpty() {
        return this->size == 0;
    }
};