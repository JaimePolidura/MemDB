#include "CounterLatch.h"

bool CounterLatch::awaitMinOrEq(int minValue, uint64_t timeoutMs) {
    if(this->counter.load(std::memory_order_consume) >= minValue){
        return true;
    }

    std::unique_lock<std::mutex> uniqueLock(this->lock);

    return this->cond.wait_for(uniqueLock,
                               std::chrono::milliseconds(timeoutMs),
                               [this, minValue](){return  this->counter.load(std::memory_order_consume) >= minValue;});
}

void CounterLatch::increase() {
    this->counter.fetch_add(1, std::memory_order_seq_cst);
    this->cond.notify_all();
}