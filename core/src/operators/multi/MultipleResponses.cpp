#include "MultipleResponses.h"

MultipleResponses::MultipleResponses(memdbNodeId_t nodeId): nodeId(nodeId) {}

void MultipleResponses::handleInitMultiResponseRequest(uint64_t requestNumber, const OperationBody &request) {
    uint64_t totalNFragmentsFromRequest = request.getArg(0).to<uint64_t>();

    this->onGoingMultiResponsesByIdLock.lock();

    this->onGoingMultiResponsesById[requestNumber] = OnGoingMultipleResponses {
        .totalNFragments = totalNFragmentsFromRequest,
        .nFragmentsReceived = 0,
    };

    this->onGoingMultiResponsesByIdLock.unlock();
}

uint64_t MultipleResponses::nextMultiResponseId() {
    return this->nodeId << 32 | this->nextMultiResponsesIdCounter.fetch_add(1);
}

bool MultipleResponses::handleFragmentRequest(uint64_t requestNumber) {
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