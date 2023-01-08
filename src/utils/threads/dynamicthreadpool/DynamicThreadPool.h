#pragma once

#include <atomic>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <numeric>

#include "utils/datastructures/queue/BlockingQueue.h"
#include "./DynamicThreadPoolWorker.h"

class DynamicThreadPool {
private:
    std::vector<std::shared_ptr<DynamicThreadPoolWorker>> workers;
    std::atomic_uint64_t numberTaskEnqueued;
    std::mutex autoScaleLock;
    std::string name;
    int inspectionPerTaskEnqueued;
    int maxThreads;
    int minThreads;
    uint8_t loadFactor;

public:
    DynamicThreadPool(uint8_t loadFactor, int maxThreadsCons, int minThreadsCons, int inspectionPerTaskEnqueuedCons, const std::string& name = ""):
            loadFactor(loadFactor), maxThreads(maxThreadsCons), minThreads(minThreadsCons), inspectionPerTaskEnqueued(inspectionPerTaskEnqueuedCons),
            name(std::move(name)) {

        this->start();
    }

    void submit(Task task) {
        this->numberTaskEnqueued++;

        this->sendTaskToWorker(task);

        if(this->numberTaskEnqueued % this->inspectionPerTaskEnqueued == 0)
            this->makeAutoscale();
    }

    void stop() {
        for(const std::shared_ptr<DynamicThreadPoolWorker>& worker : this->workers)
            worker->stop();
    }

    int getNumberWorkers() {
        return this->workers.size();
    }

private:
    void sendTaskToWorker(Task task) {
        std::shared_ptr<DynamicThreadPoolWorker> worker = * std::min_element(
                this->workers.begin(),
                this->workers.end(),
                [](const std::shared_ptr<DynamicThreadPoolWorker>& a, const std::shared_ptr<DynamicThreadPoolWorker>& b) -> bool {
                    return a->enqueuedTasks() > b->enqueuedTasks();
                });

        bool taskEnqueued = worker->enqueue(task);
        if(!taskEnqueued){ //The worker have been removed
            sendTaskToWorker(task);
        }
    }

    void start() {
        this->createWorkers(this->minThreads);
    }

    void makeAutoscale() {
        if(!this->autoScaleLock.try_lock())
            return;

        int totalTask = std::accumulate(
                this->workers.begin(),
                this->workers.end(),
                0,
                [](int total, const std::shared_ptr<DynamicThreadPoolWorker>& act){ return total + act->enqueuedTasks();});

        float actualLoadFactor = totalTask / this->workers.size();
        int newNumberOfWorkersNotAdjusted = totalTask / this->loadFactor;
        int newNumberOfWorkersAdjusted = newNumberOfWorkersNotAdjusted < this->minThreads ? this->minThreads :
                                         (newNumberOfWorkersNotAdjusted > this->maxThreads ? this->maxThreads : newNumberOfWorkersNotAdjusted);
        int numberWorkersToChange = newNumberOfWorkersAdjusted - this->workers.size();

        if(numberWorkersToChange > 0)
            this->createWorkers(numberWorkersToChange);
        else if(numberWorkersToChange < 0)
            this->deleteWorkers(numberWorkersToChange * -1);

        this->numberTaskEnqueued = 0;

        this->autoScaleLock.unlock();
    }

    void createWorkers(int numberWorkers) {
        for (int i = 0; i < numberWorkers; ++i){
            std::shared_ptr<DynamicThreadPoolWorker> newWorker = std::make_shared<DynamicThreadPoolWorker>(this->name);
            this->workers.push_back(newWorker);
            newWorker->startThread();
        }
    }

    void deleteWorkers(int numberWorkers) {
        for(int i = 0; i < numberWorkers; i++) {
            auto worker = this->workers.at(i);

            this->workers.erase(this->workers.begin() + i);
            worker->stop();
            i--;
        }
    }
};
