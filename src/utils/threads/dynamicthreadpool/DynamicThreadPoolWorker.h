#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <vector>
#include "utils/datastructures/queue/BlockingQueue.h"
#include "utils/threads/Exchanger.h"

enum WorkerState {
    ACTIVE, INACTIVE
};

class DynamicThreadPoolWorker {
private:
    std::shared_ptr<BlockingQueue<std::function<void()>>> pendingTasks;
    Exchanger<std::function<void()>> fastPath;
    std::thread thread;
    volatile WorkerState state;
    volatile bool isStoped;

public:
    DynamicThreadPoolWorker(std::shared_ptr<BlockingQueue<std::function<void()>>> pendingTasks):
        pendingTasks(std::move(pendingTasks)), isStoped(false), state(INACTIVE) {}

    void startThreadWithInitialTask(std::shared_ptr<std::function<void()>> initialTask) {
        this->fastPath.asyncEnqueue(std::move(initialTask));
        this->thread = std::thread([this]{this->run();});
    }

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