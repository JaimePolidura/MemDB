#pragma once

#include "shared.h"

#include "utils/datastructures/queue/BlockingQueue.h"

using Task = std::function<void()>;

class Worker {
private:
    std::shared_ptr<BlockingQueue<Task>> pendingTasks;
    volatile bool isStoped;
    std::thread thread;
public:
    Worker(): pendingTasks(std::make_shared<BlockingQueue<Task>>()), isStoped(false) {}

    void startThread() {
        this->thread = std::thread([this]{this->run();});
    }

    void run() {
        while (true) {
            if(this->isStoped && this->pendingTasks->getSize() == 0)
                return;

            try{
                Task task = std::move(this->pendingTasks->dequeue());

                task();
            }catch (const InterruptedQueueException e) {
                //Ignore, worker has been stoped
            }
        }
    }

    bool enqueue(Task task) {
        if(this->isStoped)
            return false;

        this->pendingTasks->enqueue(task);

        return true;
    }

    void stop() {
        this->isStoped = true;
        this->pendingTasks->stopNow();
    }

    int enqueuedTasks() const {
        return this->pendingTasks->getSize();
    }
};