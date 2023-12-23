#include "Counter.h"

Counter::Counter(memdbNodeId_t selfNodeId, uint32_t nNodes): nNodes(nNodes), nIncrements(new int64_t[nNodes]),
    nDecrements(new int64_t[nNodes]), selfNodeId(selfNodeId), arrayNodeIndexMapper(std::make_shared<CounterArrayIndexMapper>()) {
    memset(nIncrements, 0, sizeof(int64_t) * nNodes);
    memset(nDecrements, 0, sizeof(int64_t) * nNodes);
}

int64_t Counter::increment() {
    return ++this->nIncrements[this->getIndexByNode(this->selfNodeId)];
}

int64_t Counter::decrement() {
    return ++this->nDecrements[this->getIndexByNode(this->selfNodeId)];
}

void Counter::syncIncrement(int64_t valueToSync, memdbNodeId_t otherNodeId) {
    int indexIncrements = this->getIndexByNode(otherNodeId);
    this->maybeResizeCounters(indexIncrements);

    if(valueToSync > *(this->nIncrements + indexIncrements)) {
        *(this->nIncrements + indexIncrements) = valueToSync;
    }
}

void Counter::syncDecrement(int64_t valueToSync, memdbNodeId_t otherNodeId) {
    int indexDecrements = this->getIndexByNode(otherNodeId);
    this->maybeResizeCounters(indexDecrements);

    if(valueToSync > *(this->nDecrements + indexDecrements)) {
        *(this->nDecrements + indexDecrements) = valueToSync;
    }
}

int64_t Counter::count() const {
    int64_t actualValue = 0;

    for(int i = 0; i < this->nNodes; i++) {
        actualValue += this->nIncrements[i];
    }
    for(int i = 0; i < this->nNodes; i++) {
        actualValue -= this->nDecrements[i];
    }

    return actualValue;
}

ReplicateCounterResponse Counter::onReplicationCounter(ReplicateCounterRequest request) {
    this->maybeResizeCounters(this->getIndexByNode(request.otherNodeId));

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
        int64_t * newPtrIncrements = new int64_t[otherNodeId + 1];
        memset(newPtrIncrements, 0, sizeof(int64_t) * (otherNodeId + 1));
        memcpy(newPtrIncrements, this->nIncrements, this->nNodes * sizeof(int64_t));

        int64_t * newPtrDecrements = new int64_t[otherNodeId + 1];
        memset(newPtrDecrements, 0, sizeof(int64_t) * (otherNodeId + 1));
        memcpy(newPtrDecrements, this->nDecrements, this->nNodes * sizeof(int64_t));

        this->nNodes = otherNodeId + 1;
        this->nIncrements = newPtrIncrements;
        this->nDecrements = newPtrDecrements;
    }
}

void Counter::updateCounterFromReplication(ReplicateCounterRequest request) {
    if(request.isIncrement) {
        int64_t * valuePtr = this->nIncrements + getIndexByNode(request.otherNodeId);
        if(*valuePtr < request.newValue ) {
            *(this->nIncrements + getIndexByNode(request.otherNodeId)) = request.newValue;
        }
    } else {
        int64_t * valuePtr = this->nDecrements + getIndexByNode(request.otherNodeId);
        if(*valuePtr < request.newValue ) {
            *(this->nDecrements + getIndexByNode(request.otherNodeId)) = request.newValue;
        }
    }
}

std::pair<int64_t, int64_t> Counter::getLastSeen(memdbNodeId_t nodeId) const {
    if(nodeId >= nNodes) {
        return {0, 0};
    }

    int64_t lastSennIncrement = *(this->nIncrements + getIndexByNode(nodeId));
    int64_t lastSennDecrement = *(this->nDecrements + getIndexByNode(nodeId));

    return {lastSennIncrement, lastSennDecrement};
}

int Counter::getIndexByNode(memdbNodeId_t nodeId) const {
    return this->arrayNodeIndexMapper->getIndex(nodeId);
}
