#include "utils/threads/pool/Worker.h"

Worker::Worker(): pendingTasks(std::make_shared<BlockingQueue<Task>>()), isStopped(false) {}

void Worker::startThread() {
    this->thread = std::thread([this]{ this->pollPendingTasks();});
}

void Worker::pollPendingTasks() {
    while (true) {
        if(this->isStopped && this->pendingTasks->getSize() == 0)
            return;

        try{
            Task task = std::move(this->pendingTasks->dequeue());

            task();
        }catch (const InterruptedQueueException& e) {
            //Ignore, worker has been stoped
        }
    }
}

bool Worker::enqueue(Task task) {
    if(this->isStopped)
        return false;

    this->pendingTasks->enqueue(task);

    return true;
}

void Worker::stop() {
    this->isStopped = true;
    this->pendingTasks->stopNow();
}
