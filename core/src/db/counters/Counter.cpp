#include "Counter.h"

Counter::Counter(memdbNodeId_t selfNodeId, uint32_t nNodes): nNodes(nNodes), nIncrements(new uint64_t[nNodes]), nDecrements(new uint64_t[nNodes]), selfNodeId(selfNodeId) {}

uint64_t Counter::increment() {
    return ++this->nIncrements[this->selfNodeId];
}

uint64_t Counter::decrement() {
    return --this->nIncrements[this->selfNodeId];
}

void Counter::syncIncrement(uint64_t valueToSync, memdbNodeId_t otherNodeId) {
    if(valueToSync > *(this->nIncrements + otherNodeId)) {
        *(this->nIncrements + otherNodeId) = valueToSync;
    }
}

void Counter::syncDecrement(uint64_t valueToSync, memdbNodeId_t otherNodeId) {
    if(valueToSync > *(this->nDecrements + otherNodeId)) {
        *(this->nDecrements + otherNodeId) = valueToSync;
    }
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

ReplicateCounterResponse Counter::onReplicationCounter(ReplicateCounterRequest request) {
    if(request.otherNodeId >= this->nNodes) {
        uint64_t * newPtrIncrements = new uint64_t[request.otherNodeId + 1];
        memset(newPtrIncrements, 0, request.otherNodeId + 1);
        memcpy(newPtrIncrements, this->nIncrements, this->nNodes);

        uint64_t * newPtrDecrements = new uint64_t[request.otherNodeId + 1];
        memset(newPtrDecrements, 0, request.otherNodeId + 1);
        memcpy(newPtrDecrements, this->nDecrements, this->nNodes);

        this->nNodes = request.otherNodeId + 1;
        this->nIncrements = newPtrIncrements;
        this->nDecrements = newPtrDecrements;
    }

    ReplicateCounterResponse response{.needsIncrementSync = false, .needsDecrementSync = false};

    updateCounterFromReplication(request);

    if(request.lastSeenSelfIncrement < this->nIncrements[this->selfNodeId]) {
        response.needsIncrementSync = true;
        response.nIncrementToSync = this->nIncrements[this->selfNodeId];
    }

    if(request.lastSeenSelfDecrement < this->nDecrements[this->selfNodeId]) {
        response.needsDecrementSync = true;
        response.nDecrementToSync = this->nDecrements[this->selfNodeId];
    }

    return response;
}

void Counter::updateCounterFromReplication(ReplicateCounterRequest request) {
    if(request.isIncrement) {
        uint64_t * valuePtr = this->nIncrements + request.otherNodeId;
        if(*valuePtr < request.newValue ) {
            *(this->nIncrements + request.otherNodeId) = request.newValue;
        }
    } else {
        uint64_t * valuePtr = this->nDecrements + request.otherNodeId;
        if(*valuePtr < request.newValue ) {
            *(this->nDecrements + request.otherNodeId) = request.newValue;
        }
    }
}

std::pair<uint64_t, uint64_t> Counter::getLastSeen(memdbNodeId_t nodeId) const {
    if(nodeId >= nNodes) {
        return {0, 0};
    }

    uint64_t lastSennIncrement = *(this->nIncrements + nodeId);
    uint64_t lastSennDecrement = *(this->nDecrements + nodeId);

    return {lastSennIncrement, lastSennDecrement};
}