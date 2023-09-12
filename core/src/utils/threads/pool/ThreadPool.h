#pragma once

#include "shared.h"
#include "utils/threads/pool/Worker.h"

thread_local uint64_t counter;

class ThreadPool {
private:
    std::vector<std::shared_ptr<Worker>> workers;
    
public:
    explicit ThreadPool(int threads);

    ThreadPool() = default;

    void submit(Task task);

    void stop();

private:
    void createWorkers(int numberWorkers);
};
