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
        pendingTasks(std::move(pendingTasks)), isStoped(true) {}

    void startThreadWithInitialTask(const std::function<void()>& initialTask) {
        this->fastPath.asyncEnqueue(initialTask);
        this->thread = std::thread([this]{this->run();});
    }

    void startThread() {
        this->thread = std::thread([this]{this->run();});
    }

    void run() {
        while (!this->isStoped) {
            this->state = INACTIVE;
            std::function<void()> task = this->pendingTasks->dequeue();
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