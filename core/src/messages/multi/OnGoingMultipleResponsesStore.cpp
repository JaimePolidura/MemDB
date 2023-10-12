#include "OnGoingMultipleResponsesStore.h"

OnGoingMultipleResponsesStore::OnGoingMultipleResponsesStore(memdbNodeId_t nodeId): nodeId(nodeId) {}

void OnGoingMultipleResponsesStore::registerIncomingMultiInit(uint64_t multiId, iterator_t<std::vector<uint8_t>> iterator) {
    this->onGoingMultiResponsesById[multiId] = OnGoingMultipleResponsesSender{
        .iterator = iterator,
        .totalNFragments = iterator->totalSize(),
        .nFragmentsSent = 0,
    };
}

iterator_t<std::vector<uint8_t>> OnGoingMultipleResponsesStore::getSenderIteratorByMultiId(uint64_t multiId) {
    return this->onGoingMultiResponsesById[multiId].iterator;
}

void OnGoingMultipleResponsesStore::markFragmentSend(uint64_t multiId) {
    OnGoingMultipleResponsesSender& onGoingMultiResponse = this->onGoingMultiResponsesById[multiId];
    onGoingMultiResponse.nFragmentsSent++;

    if(onGoingMultiResponse.nFragmentsSent >= onGoingMultiResponse.totalNFragments){
        this->onGoingMultiResponsesById.erase(multiId);
    }
}

uint64_t OnGoingMultipleResponsesStore::nextMultiResponseId() {
    return this->nodeId << 32 | this->nextMultiIdCounter.fetch_add(1);
}