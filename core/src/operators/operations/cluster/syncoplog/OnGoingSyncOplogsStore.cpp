#include "OnGoingSyncOplogsStore.h"

OnGoingSyncOplogsStore::OnGoingSyncOplogsStore(memdbNodeId_t nodeId): selfNodeId(nodeId) {}

void OnGoingSyncOplogsStore::registerSyncOplogIterator(uint64_t multiId, oplogSegmentIterator_t segmentIterator) {
    std::lock_guard<std::mutex> guard(this->onGoingSyncOplogsByIdLock);

    this->onGoingSyncOplogsById[multiId] = OnGoingSyncOplog{
        .iterator = segmentIterator,
        .totalNFragments = segmentIterator->totalSize(),
        .nFragmentsSent = 0,
    };
}

std::optional<oplogSegmentIterator_t> OnGoingSyncOplogsStore::getSenderIteratorById(uint64_t syncOplogId) {
    std::lock_guard<std::mutex> guard(this->onGoingSyncOplogsByIdLock);

    if(this->onGoingSyncOplogsById.contains(syncOplogId)) {
        this->onGoingSyncOplogsById[syncOplogId].iterator;
    } else {
        return std::nullopt;
    }
}

void OnGoingSyncOplogsStore::markSegmentAsSent(uint64_t multiId) {
    std::lock_guard<std::mutex> guard(this->onGoingSyncOplogsByIdLock);

    OnGoingSyncOplog& onGoingMultiResponse = this->onGoingSyncOplogsById[multiId];
    onGoingMultiResponse.nFragmentsSent++;

    if(onGoingMultiResponse.nFragmentsSent >= onGoingMultiResponse.totalNFragments){
        this->onGoingSyncOplogsById.erase(multiId);
    }
}

uint64_t OnGoingSyncOplogsStore::nextSyncOplogId() {
    return this->selfNodeId << 32 | this->nextSyncOplogIdCounter.fetch_add(1);
}