#pragma once

#include "shared.h"

/**
 * HEAD             TAIL
 * A --> B --> C --> D
 */

template<typename T>
class BufferOneConsumerNode {
public:
    std::atomic<BufferOneConsumerNode *> next;
    T value;

public:
    BufferOneConsumerNode(const T& value): value(std::move(value)) {}
};

template<typename T>
class BufferOneConsumer {
    std::atomic<BufferOneConsumerNode<T> *> head;

    void enqueue(const T& value) {
        BufferOneConsumerNode<T> * newNode = new BufferOneConsumerNode<T>(value);

        while(true){
            if(this->head == nullptr && this->head.compare_exchange_strong(this->head, newNode)) {
                //Notify dequeuer
                return;
            }
            if(this->appendNodeToList(newNode))
                return;
        }
    }

    T dequeue() {
        if(this->head == nullptr){
            //Wait
        }

        T value = this->head->value;
        this->head = this->head->next;

        return value;
    }
private:
    bool appendNodeToList(BufferOneConsumerNode<T> * newNode) {
        BufferOneConsumerNode<T> * lastNode = this->getLastNodeEnqueued();

        while(lastNode != nullptr && lastNode->next.compare_exchange_strong(lastNode->next, newNode)){
            lastNode = lastNode->next;
        }

        return lastNode != nullptr;
    }

    BufferOneConsumerNode<T> * getLastNodeEnqueued() {
        std::atomic<BufferOneConsumerNode<T> *> last = this->head;

        do {
            last = last->next;
        }while(head != nullptr);

        return last;
    }
};