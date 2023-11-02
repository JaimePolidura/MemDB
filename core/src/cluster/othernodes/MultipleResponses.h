#pragma once

#include "utils/std/Result.h"
#include "messages/response/Response.h"
#include "utils/threads/CounterLatch.h"

class MultipleResponses {
private:
    std::map<memdbNodeId_t, Response> responses{};
    std::mutex responsesLock{};
    CounterLatch latch{};
    int nTotalRequest;

    friend class MultipleResponsesNotifier;
public:

    MultipleResponses(int nTotalRequest);

    bool waitForQuorum(uint64_t timeoutMs);

    std::map<memdbNodeId_t, Response> getResponses();

    bool allResponsesSuccessful();
};

class MultipleResponsesNotifier {
private:
    std::shared_ptr<MultipleResponses> multipleResponse;

public:
    MultipleResponsesNotifier(std::shared_ptr<MultipleResponses> multipleResponse);

    void addResponse(memdbNodeId_t nodeId, const Response& response);
};

using multipleResponses_t = std::shared_ptr<MultipleResponses>;