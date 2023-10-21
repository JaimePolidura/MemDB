#include "OnGoingSyncOplogsStore.h"

OnGoingSyncOplogsStore::OnGoingSyncOplogsStore(memdbNodeId_t nodeId): selfNodeId(nodeId) {}

void OnGoingSyncOplogsStore::registerSyncOplogIterator(uint64_t syncId, oplogSegmentIterator_t segmentIterator) {
    std::lock_guard<std::mutex> guard(this->onGoingSyncOplogsByIdLock);

    if(!this->onGoingSyncOplogsById.contains(syncId)){
        this->onGoingSyncOplogsById[syncId] = OnGoingSyncOplog{
                .iterator = segmentIterator,
                .totalNFragments = segmentIterator->totalSize(),
                .nFragmentsSent = 0,
        };
    }
}

std::optional<oplogSegmentIterator_t> OnGoingSyncOplogsStore::getSenderIteratorById(uint64_t syncOplogId) {
    std::lock_guard<std::mutex> guard(this->onGoingSyncOplogsByIdLock);

    return this->onGoingSyncOplogsById.contains(syncOplogId) ?
           std::optional<oplogSegmentIterator_t>{this->onGoingSyncOplogsById[syncOplogId].iterator} :
           std::nullopt;
}

void OnGoingSyncOplogsStore::markSegmentAsSent(uint64_t syncId) {
    std::lock_guard<std::mutex> guard(this->onGoingSyncOplogsByIdLock);

    OnGoingSyncOplog& onGoingMultiResponse = this->onGoingSyncOplogsById[syncId];
    onGoingMultiResponse.nFragmentsSent++;

    if(onGoingMultiResponse.nFragmentsSent >= onGoingMultiResponse.totalNFragments){
        this->onGoingSyncOplogsById.erase(syncId);
    }
}

void OnGoingSyncOplogsStore::removeBySyncId(uint64_t syncId) {
    this->onGoingSyncOplogsById.erase(syncId);
}

uint64_t OnGoingSyncOplogsStore::nextSyncOplogId() {
    return ((uint64_t) this->selfNodeId) << 32 | this->nextSyncOplogIdCounter.fetch_add(1);
}