#include "Counter.h"

Counter::Counter(memdbNodeId_t selfNodeId, uint32_t nNodes): nNodes(nNodes), nIncrements(new uint64_t[nNodes]),
    nDecrements(new uint64_t[nNodes]), selfNodeId(selfNodeId), arrayNodeIndexMapper(std::make_shared<CounterArrayIndexMapper>()) {
    memset(nIncrements, 0, sizeof(uint64_t) * nNodes);
    memset(nDecrements, 0, sizeof(uint64_t) * nNodes);
}

uint64_t Counter::increment() {
    return ++this->nIncrements[this->getIndexByNode(this->selfNodeId)];
}

uint64_t Counter::decrement() {
    return ++this->nDecrements[this->getIndexByNode(this->selfNodeId)];
}

void Counter::syncIncrement(uint64_t valueToSync, memdbNodeId_t otherNodeId) {
    this->maybeResizeCounters(otherNodeId);

    if(valueToSync > *(this->nIncrements + this->getIndexByNode(otherNodeId))) {
        *(this->nIncrements + this->getIndexByNode(otherNodeId)) = valueToSync;
    }
}

void Counter::syncDecrement(uint64_t valueToSync, memdbNodeId_t otherNodeId) {
    this->maybeResizeCounters(otherNodeId);

    if(valueToSync > *(this->nDecrements + this->getIndexByNode(otherNodeId))) {
        *(this->nDecrements + this->getIndexByNode(otherNodeId)) = valueToSync;
    }
}

uint64_t Counter::count() const {
    uint64_t actualValue = 0;

    for(int i = 0; i < this->nNodes; i++) {
        actualValue += this->nIncrements[i];
    }
    for(int i = 0; i < this->nNodes; i++) {
        uint64_t actualToDecrement = this->nDecrements[i];
        if(actualToDecrement >= actualValue) {
            actualValue = 0;
            break;
        } else {
            actualValue -= actualToDecrement;
        }
    }

    return actualValue;
}

ReplicateCounterResponse Counter::onReplicationCounter(ReplicateCounterRequest request) {
    this->maybeResizeCounters(request.otherNodeId);

    ReplicateCounterResponse response{.needsIncrementSync = false, .needsDecrementSync = false};

    updateCounterFromReplication(request);

    if(request.lastSeenSelfIncrement < this->nIncrements[this->getIndexByNode(this->selfNodeId)]) {
        response.needsIncrementSync = true;
        response.nIncrementToSync = this->nIncrements[this->getIndexByNode(this->selfNodeId)];
    }

    if(request.lastSeenSelfDecrement < this->nDecrements[this->getIndexByNode(this->selfNodeId)]) {
        response.needsDecrementSync = true;
        response.nDecrementToSync = this->nDecrements[this->getIndexByNode(this->selfNodeId)];
    }

    return response;
}

void Counter::maybeResizeCounters(memdbNodeId_t otherNodeId) {
    if(otherNodeId >= this->nNodes) {
        uint64_t * newPtrIncrements = new uint64_t[otherNodeId + 1];
        memset(newPtrIncrements, 0, sizeof(uint64_t) * (otherNodeId + 1));
        memcpy(newPtrIncrements, this->nIncrements, this->nNodes * sizeof(uint64_t));

        uint64_t * newPtrDecrements = new uint64_t[otherNodeId + 1];
        memset(newPtrDecrements, 0, sizeof(uint64_t) * (otherNodeId + 1));
        memcpy(newPtrDecrements, this->nDecrements, this->nNodes * sizeof(uint64_t));

        this->nNodes = otherNodeId + 1;
        this->nIncrements = newPtrIncrements;
        this->nDecrements = newPtrDecrements;
    }
}

void Counter::updateCounterFromReplication(ReplicateCounterRequest request) {
    if(request.isIncrement) {
        uint64_t * valuePtr = this->nIncrements + getIndexByNode(request.otherNodeId);
        if(*valuePtr < request.newValue ) {
            *(this->nIncrements + getIndexByNode(request.otherNodeId)) = request.newValue;
        }
    } else {
        uint64_t * valuePtr = this->nDecrements + getIndexByNode(request.otherNodeId);
        if(*valuePtr < request.newValue ) {
            *(this->nDecrements + getIndexByNode(request.otherNodeId)) = request.newValue;
        }
    }
}

std::pair<uint64_t, uint64_t> Counter::getLastSeen(memdbNodeId_t nodeId) const {
    if(nodeId >= nNodes) {
        return {0, 0};
    }

    uint64_t lastSennIncrement = *(this->nIncrements + getIndexByNode(nodeId));
    uint64_t lastSennDecrement = *(this->nDecrements + getIndexByNode(nodeId));

    return {lastSennIncrement, lastSennDecrement};
}

int Counter::getIndexByNode(memdbNodeId_t nodeId) const {
    return this->arrayNodeIndexMapper->getIndex(nodeId);
}
