#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <vector>
#include <string>

#include "utils/datastructures/queue/BlockingQueue.h"

using Task = std::function<void()>;

enum WorkerState {
    ACTIVE, INACTIVE
};

class DynamicThreadPoolWorker {
private:
    std::shared_ptr<BlockingQueue<Task>> pendingTasks;
    std::thread thread;
    volatile WorkerState state;
    volatile bool isStoped;
    std::string name;

public:
    DynamicThreadPoolWorker(std::string name):
        pendingTasks(std::make_shared<BlockingQueue<Task>>()), isStoped(false), state(INACTIVE), name(name) {}

    void startThread() {
        this->thread = std::thread([this]{this->run();});
    }

    void run() {
        while (true) {
            if(this->isStoped && this->pendingTasks->getSize() == 0)
                return;

            try{
                Task task = std::move(this->pendingTasks->dequeue());

                this->state = ACTIVE;
                task();
                this->state = INACTIVE;
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

    WorkerState getState() const {
        return this->state;
    }
};