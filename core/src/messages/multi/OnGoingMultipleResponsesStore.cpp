#include "OnGoingMultipleResponsesStore.h"

OnGoingMultipleResponsesStore::OnGoingMultipleResponsesStore(memdbNodeId_t nodeId): nodeId(nodeId) {}

void OnGoingMultipleResponsesStore::handleInitMultiResponseRequest(uint64_t requestNumber, uint64_t totalNFragmentsFromRequest) {
    this->onGoingMultiResponsesByIdLock.lock();

    this->onGoingMultiResponsesById[requestNumber] = OnGoingMultipleResponses {
        .totalNFragments = totalNFragmentsFromRequest,
        .nFragmentsReceived = 0,
    };

    this->onGoingMultiResponsesByIdLock.unlock();
}

uint64_t OnGoingMultipleResponsesStore::nextMultiResponseId() {
    return this->nodeId << 32 | this->nextMultiResponsesIdCounter.fetch_add(1);
}

bool OnGoingMultipleResponsesStore::handleFragmentRequest(uint64_t requestNumber) {
    bool allFragmentsReceived = false;

    this->onGoingMultiResponsesByIdLock.lock();

    if(this->onGoingMultiResponsesById.contains(requestNumber)){
        OnGoingMultipleResponses& onGoingMultiResponses = this->onGoingMultiResponsesById.at(requestNumber);
        onGoingMultiResponses.nFragmentsReceived++;
        allFragmentsReceived = onGoingMultiResponses.nFragmentsReceived >= onGoingMultiResponses.totalNFragments;
        this->onGoingMultiResponsesById.erase(requestNumber);
    }

    this->onGoingMultiResponsesByIdLock.unlock();

    return allFragmentsReceived;
}