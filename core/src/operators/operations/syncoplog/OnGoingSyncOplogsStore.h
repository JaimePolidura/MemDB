#pragma once

#include "persistence/OplogIterator.h"
#include "messages/request/Request.h"
#include "utils/Iterator.h"
#include "shared.h"

struct OnGoingSyncOplog {
    oplogIterator_t iterator;
    uint64_t totalNFragments;
    uint64_t nFragmentsSent;
};

/**
 * We use RequestNumber as the multi-request Id
 */
class OnGoingSyncOplogsStore {
private:
    std::map<uint64_t, OnGoingSyncOplog> onGoingSyncOplogsById{};
    std::mutex onGoingSyncOplogsByIdLock{};

    std::atomic_uint32_t nextSyncOplogIdCounter; //Used by initiator / receiver
    memdbNodeId_t selfNodeId;

public:
    explicit OnGoingSyncOplogsStore(memdbNodeId_t nodeId);

    void registerSyncOplogIterator(uint64_t syncId, oplogIterator_t segmentIterator);

    std::optional<oplogIterator_t> getSenderIteratorById(uint64_t id);

    void markSegmentAsSent(uint64_t syncId);

    void removeBySyncId(uint64_t syncId);

    uint64_t nextSyncOplogId();
};

using onGoingSyncOplogs_t = std::shared_ptr<OnGoingSyncOplogsStore>;
