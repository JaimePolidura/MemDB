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
    Worker();

    void startThread();

    void run();

    bool enqueue(Task task);

    void stop();

    int enqueuedTasks() const;
};