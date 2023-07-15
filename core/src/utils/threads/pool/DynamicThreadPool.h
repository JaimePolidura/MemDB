#pragma once

#include "shared.h"
#include "utils/threads/pool/Worker.h"

class DynamicThreadPool {
private:
    std::vector<std::shared_ptr<Worker>> workers;
    std::uint64_t numberTaskEnqueued;
    std::mutex autoScaleLock;
    int inspectionPerTaskEnqueued;
    int maxThreads;
    int minThreads;
    uint8_t loadFactor;
    std::atomic_uint64_t nextWorker;

public:
    DynamicThreadPool(uint8_t loadFactor, int maxThreadsCons, int minThreadsCons, int inspectionPerTaskEnqueuedCons);

    DynamicThreadPool() = default;

    void submit(Task task);

    void stop();

    int getNumberWorkers();

private:
    void sendTaskToWorker(Task task);

    std::shared_ptr<Worker> getWorker();

    void start();

    void makeAutoscale();

    void createWorkers(int numberWorkers);

    void deleteWorkers(int numberWorkers);
};
