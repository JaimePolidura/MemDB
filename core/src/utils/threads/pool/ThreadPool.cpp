#include "utils/threads/pool/ThreadPool.h"

ThreadPool::ThreadPool(int threads): counter(0) {
    this->createWorkers(threads);
}

void ThreadPool::submit(Task task) {
    if(this->counter == 0){
        this->counter = std::hash<std::thread::id>{}(std::this_thread::get_id());
    }

    auto workerIndex = ++this->counter % this->workers.size();
    auto worker = this->workers[workerIndex];

    worker->enqueue(task);
}

void ThreadPool::stop() {
    for(const std::shared_ptr<Worker>& worker : this->workers) {
        worker->stop();
    }
}

void ThreadPool::createWorkers(int numberWorkers) {
    for (int i = 0; i < numberWorkers; ++i){
        std::shared_ptr<Worker> newWorker = std::make_shared<Worker>();
        this->workers.push_back(newWorker);
        newWorker->startThread();
    }
}
