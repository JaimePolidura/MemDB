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
    DynamicThreadPool(uint8_t loadFactor, int maxThreadsCons, int minThreadsCons, int inspectionPerTaskEnqueuedCons):
            loadFactor(loadFactor), maxThreads(maxThreadsCons), minThreads(minThreadsCons),
            inspectionPerTaskEnqueued(inspectionPerTaskEnqueuedCons), nextWorker(0) {

        this->start();
    }

    void submit(Task task) {
        this->numberTaskEnqueued++;

        this->sendTaskToWorker(task);

        if((this->numberTaskEnqueued % this->inspectionPerTaskEnqueued) == 0)
            this->makeAutoscale();
    }

    void stop() {
        for(const std::shared_ptr<Worker>& worker : this->workers)
            worker->stop();
    }

    int getNumberWorkers() {
        return this->workers.size();
    }

private:
    void sendTaskToWorker(Task task) {
        std::shared_ptr<Worker> worker = this->getWorker();

        bool taskEnqueued = worker->enqueue(task);
        if(!taskEnqueued){ //The worker have been removed
            sendTaskToWorker(task);
        }
    }

    std::shared_ptr<Worker> getWorker() {
        this->nextWorker++;

        return this->workers.at(this->nextWorker % this->workers.size());
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
                [](int total, const std::shared_ptr<Worker>& act){ return total + act->enqueuedTasks();});

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
            std::shared_ptr<Worker> newWorker = std::make_shared<Worker>();
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
