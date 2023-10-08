#pragma once

#include "messages/request/Request.h"
#include "shared.h"

struct OnGoingMultipleResponses {
    uint64_t totalNFragments;
    uint64_t nFragmentsReceived;
};

/**
 * We use RequestNumber as the multi-request Id
 */
class MultipleResponses {
private:
    std::map<uint64_t, OnGoingMultipleResponses> onGoingMultiResponsesById{};
    std::mutex onGoingMultiResponsesByIdLock{};

    std::atomic_uint32_t nextMultiResponsesIdCounter;
    memdbNodeId_t nodeId;

public:
    explicit MultipleResponses(memdbNodeId_t nodeId);

    void handleInitMultiResponseRequest(uint64_t requestNumber, const OperationBody &request);

    bool handleFragmentRequest(uint64_t requestNumber);

    uint64_t nextMultiResponseId();
};

using multipleResponses_t = std::shared_ptr<MultipleResponses>;
