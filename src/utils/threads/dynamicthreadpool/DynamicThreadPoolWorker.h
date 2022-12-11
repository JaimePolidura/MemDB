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
    WorkerState state;
    bool isStoped;

public:
    DynamicThreadPoolWorker(std::shared_ptr<BlockingQueue<std::function<void()>>> pendingTasks):
        pendingTasks(std::move(pendingTasks)), isStoped(false) {}

    void startThreadWithInitialTask(std::shared_ptr<std::function<void()>> initialTask) {
        this->fastPath.asyncEnqueue(std::move(initialTask));
        this->thread = std::thread([this]{this->run();});
    }

    void startThread() {
        this->thread = std::thread([this]{this->run();});
    }

    void joinThread() {
        this->thread;
    }

    void run() {
        while (!this->isStoped) {
            this->state = INACTIVE;

            std::function<void()> task = this->pendingTasks->dequeue();
            if(this->isStoped)
                return;

            this->state = ACTIVE;

            task();
        }
    }

    void stop() {
        this->isStoped = true;
    }

    WorkerState getState() {
        return this->state;
    }
};