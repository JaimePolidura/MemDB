#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <vector>
#include "utils/datastructures/queue/BlockingQueue.h"

enum WorkerState {
    ACTIVE, INACTIVE
};

class DynamicThreadPoolWorker {
private:
    std::shared_ptr<BlockingQueue<std::function<void()>>> pendingTasks;
    std::thread thread;
    volatile WorkerState state;
    volatile bool isStoped;

public:
    DynamicThreadPoolWorker(std::shared_ptr<BlockingQueue<std::function<void()>>> pendingTasks):
        pendingTasks(std::move(pendingTasks)), isStoped(false), state(INACTIVE) {}

    void startThread() {
        this->thread = std::thread([this]{this->run();});
    }

    void run() {
        while (!this->isStoped) {
            std::function<void()> task = this->pendingTasks->dequeue();
            if(this->isStoped)
                return;

            this->state = ACTIVE;
            task();
            this->state = INACTIVE;
        }
    }

    void stop() {
        this->isStoped = true;
    }

    WorkerState getState() {
        return this->state;
    }
};