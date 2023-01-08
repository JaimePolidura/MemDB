#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>

#include "utils/datastructures/queue/BlockingQueue.h"
#include "./DynamicThreadPoolWorker.h"

class DynamicThreadPool {
private:
    std::shared_ptr<BlockingQueue<std::function<void()>>> pendingTask;
    std::vector<std::shared_ptr<DynamicThreadPoolWorker>> workers;
    std::atomic_uint64_t numberTaskEnqueued;
    std::mutex autoScaleLock;
    int inspectionPerTaskEnqueued;
    std::string name;
    float actityFactor;
    int maxThreads;
    int minThreads;

public:
    DynamicThreadPool(float activityFactorCons, int maxThreadsCons, int minThreadsCons, int inspectionPerTaskEnqueuedCons, const std::string& name = ""):
        actityFactor(activityFactorCons), maxThreads(maxThreadsCons), minThreads(minThreadsCons), inspectionPerTaskEnqueued(inspectionPerTaskEnqueuedCons),
        pendingTask(std::make_shared<BlockingQueue<std::function<void()>>>()), name(std::move(name)) {

        this->start();
    }

    void submit(const std::function<void()>& task) {
        this->numberTaskEnqueued++;

        this->pendingTask->enqueue(task);

        if(this->numberTaskEnqueued % this->inspectionPerTaskEnqueued == 0)
            this->makeAutoScaleInspection();
    }

    void stop() {
        for(const std::shared_ptr<DynamicThreadPoolWorker>& worker : this->workers)
            worker->stop();

        this->pendingTask->stopNow();
    }

    int getNumberWorkers() {
        return this->workers.size();
    }

private:
    void start() {
        this->createWorkers(this->minThreads);
    }

    void makeAutoScaleInspection() {
        if(!this->autoScaleLock.try_lock())
            return;

        int nTotalWorkers = this->workers.size();
        int nActiveWorkers = 1 + std::count_if(this->workers.begin(),
                                               this->workers.end(),
                                               [](std::shared_ptr<DynamicThreadPoolWorker>& worker){return worker->getState() == ACTIVE;});

        int newNumberOfWorkersNotAdjusted = nActiveWorkers / this->actityFactor;
        int newNumberOfWorkersAdjusted = newNumberOfWorkersNotAdjusted < this->minThreads ? this->minThreads :
                                         (newNumberOfWorkersNotAdjusted > this->maxThreads ? this->maxThreads : newNumberOfWorkersNotAdjusted);
        int numberWorkersToChange = newNumberOfWorkersAdjusted - nTotalWorkers;

        if(numberWorkersToChange > 0)
            this->createWorkers(numberWorkersToChange);
        else if(numberWorkersToChange < 0)
            this->deleteWorkers(numberWorkersToChange * -1);

        this->autoScaleLock.unlock();
    }

    void createWorkers(int numberWorkers) {
        for (int i = 0; i < numberWorkers; ++i){
            std::shared_ptr<DynamicThreadPoolWorker> newWorker = std::make_shared<DynamicThreadPoolWorker>(this->pendingTask, this->name);
            this->workers.push_back(newWorker);
            newWorker->startThread();
        }
    }

    void deleteWorkers(int numberWorkers) {
        auto wortkersIterator = this->workers.begin();

        for (int i = 0; i < numberWorkers; ++i)
            this->workers[i]->stop();

        this->workers.erase(wortkersIterator + numberWorkers);
    }
};
