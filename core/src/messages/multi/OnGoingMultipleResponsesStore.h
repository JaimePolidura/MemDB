#pragma once

#include "messages/request/Request.h"
#include "messages/multi/MultipleResponseSenderIterator.h"
#include "shared.h"

struct OnGoingMultipleResponses {
    multipleResponseSenderIterator_t iterator;
    uint64_t totalNFragments;
    uint64_t nFragmentsReceived;
};

/**
 * We use RequestNumber as the multi-request Id
 */
class OnGoingMultipleResponsesStore {
private:
    std::map<uint64_t, OnGoingMultipleResponses> onGoingMultiResponsesById{};
    std::mutex onGoingMultiResponsesByIdLock{};

    std::atomic_uint32_t nextMultiIdCounter; //Used by initiator / receiver
    memdbNodeId_t nodeId;

public:
    explicit OnGoingMultipleResponsesStore(memdbNodeId_t nodeId);

    void registerIncomingMultiInit(uint64_t multiId, multipleResponseSenderIterator_t iterator);

    uint64_t nextMultiResponseId();
};

using onGoingMultipleResponsesStore_t = std::shared_ptr<OnGoingMultipleResponsesStore>;
