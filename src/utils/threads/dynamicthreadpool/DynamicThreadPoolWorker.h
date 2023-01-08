#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <vector>
#include <string>

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
    std::string name;

public:
    DynamicThreadPoolWorker(std::shared_ptr<BlockingQueue<std::function<void()>>> pendingTasks, std::string name):
        pendingTasks(pendingTasks), isStoped(false), state(INACTIVE), name(name) {}

    void startThread() {
        this->thread = std::thread([this]{this->run();});
    }

    void run() {
        while (!this->isStoped) {
            if(this->isStoped)
                return;

            try{
                std::function<void()> task = std::move(this->pendingTasks->dequeue());

                this->state = ACTIVE;
                task();
                this->state = INACTIVE;
            }catch (const std::exception& e) {
                printf("Error\n");
            }
        }
    }

    void stop() {
        this->isStoped = true;
    }

    WorkerState getState() {
        return this->state;
    }
};