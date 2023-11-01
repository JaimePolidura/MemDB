#include "MultipleResponses.h"

MultipleResponses::MultipleResponses(int nTotalRequest): nTotalRequest(nTotalRequest) {}

bool MultipleResponses::waitForQuorum(uint64_t timeoutMs) {
    return this->latch.awaitMinOrEq(this->nTotalRequest / 2 + 1, timeoutMs);
}

std::map<memdbNodeId_t, Response> MultipleResponses::getResponses() {
    this->responsesLock.lock();
    std::map<memdbNodeId_t, Response> copyOfResponses = this->responses;
    this->responsesLock.unlock();

    return copyOfResponses;
}

MultipleResponsesNotifier::MultipleResponsesNotifier(std::shared_ptr<MultipleResponses> multipleResponse): multipleResponse(multipleResponse) {}

void MultipleResponsesNotifier::addResponse(memdbNodeId_t nodeId, const Response& response) {
    multipleResponse->responsesLock.lock();
    multipleResponse->responses.insert({nodeId, response});
    multipleResponse->latch.increase();
    multipleResponse->responsesLock.unlock();
}