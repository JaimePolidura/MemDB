#pragma once

#include "shared.h"

#include "utils/datastructures/queue/BlockingQueue.h"

using Task = std::function<void()>;

class Worker {
private:
    std::shared_ptr<BlockingQueue<Task>> pendingTasks;
    volatile bool isStopped;
    std::thread thread;

public:
    Worker();

    void startThread();

    bool enqueue(Task task);

    void stop();

private:
    void pollPendingTasks();
};