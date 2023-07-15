#include "utils/threads/pool/Worker.h"

Worker::Worker(): pendingTasks(std::make_shared<BlockingQueue<Task>>()), isStoped(false) {}

void Worker::startThread() {
    this->thread = std::thread([this]{this->run();});
}

void Worker::run() {
    while (true) {
        if(this->isStoped && this->pendingTasks->getSize() == 0)
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
    if(this->isStoped)
        return false;

    this->pendingTasks->enqueue(task);

    return true;
}

void Worker::stop() {
    this->isStoped = true;
    this->pendingTasks->stopNow();
}

int Worker::enqueuedTasks() const {
    return this->pendingTasks->getSize();
}