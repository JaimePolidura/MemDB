#include "Counter.h"

Counter::Counter(memdbNodeId_t selfNodeId, uint32_t nNodes): nNodes(nNodes), nIncrements(new uint64_t[nNodes]), nDecrements(new uint64_t[nNodes]), selfNodeId(selfNodeId) {}

void Counter::increment() {
    this->nIncrements[this->selfNodeId]++;
}

void Counter::decrement() {
    this->nIncrements[this->selfNodeId]--;
}

uint64_t Counter::count() const {
    uint64_t actualValue = 0;

    for(int i = 0; i < this->nNodes; i++) {
        actualValue += this->nIncrements[i];
    }
    for(int i = 0; i < this->nNodes; i++) {
        actualValue -= this->nDecrements[i];
    }

    return actualValue;
}
