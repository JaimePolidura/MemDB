#pragma once

#include "messages/request/Request.h"
#include "utils/Iterator.h"
#include "shared.h"

struct OnGoingMultipleResponsesSender {
    iterator_t<std::vector<uint8_t>> iterator;
    uint64_t totalNFragments;
    uint64_t nFragmentsSent;
};

/**
 * We use RequestNumber as the multi-request Id
 */
class OnGoingMultipleResponsesStore {
private:
    std::map<uint64_t, OnGoingMultipleResponsesSender> onGoingMultiResponsesById{};
    std::mutex onGoingMultiResponsesByIdLock{};

    std::atomic_uint32_t nextMultiIdCounter; //Used by initiator / receiver
    memdbNodeId_t nodeId;

public:
    explicit OnGoingMultipleResponsesStore(memdbNodeId_t nodeId);

    void registerIncomingMultiInit(uint64_t multiId, iterator_t<std::vector<uint8_t>> iterator);
    
    iterator_t<std::vector<uint8_t>> getSenderIteratorByMultiId(uint64_t multiId);

    void markFragmentSend(uint64_t multiId);

    uint64_t nextMultiResponseId();
};

using onGoingMultipleResponsesStore_t = std::shared_ptr<OnGoingMultipleResponsesStore>;
