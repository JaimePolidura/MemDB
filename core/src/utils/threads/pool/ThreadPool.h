#pragma once

#include "shared.h"
#include "utils/threads/pool/Worker.h"
#include "utils/Utils.h"
#include "utils/std/Result.h"

class ThreadPool {
private:
    std::vector<std::shared_ptr<Worker>> workers;

public:
    explicit ThreadPool(uint64_t numberThreads);

    ThreadPool() = default;

    void submit(Task task);

    template<typename V, typename E = V>
    std::future<std::result<V, E>> submitAndExecute(Task task);

    void stop();

private:
    void createWorkers(int numberWorkers);
};
