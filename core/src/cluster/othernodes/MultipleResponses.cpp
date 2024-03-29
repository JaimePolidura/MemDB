#include "MultipleResponses.h"

MultipleResponses::MultipleResponses(int nTotalRequest): nTotalRequest(nTotalRequest) {}

std::map<memdbNodeId_t, Response> MultipleResponses::getResponses() {
    this->responsesLock.lock();
    std::map<memdbNodeId_t, Response> copyOfResponses = this->responses;
    this->responsesLock.unlock();

    return copyOfResponses;
}

bool MultipleResponses::waitForSuccessfulQuorum(uint64_t timeoutMs) {
    int nTotalNodes = this->nTotalRequest + 1;
    int minQuorum = nTotalNodes / 2 + 1;

    return this->successfulResponsesLatch.awaitMinOrEq(minQuorum - 1, timeoutMs); //-1 to include our self
}

void MultipleResponses::onResponse(std::function<void(const Response&)> onResponse) {
    std::unique_lock<std::mutex> lock(this->responsesLock);
    this->onResponsesCallbacks.push_back(onResponse);
}

bool MultipleResponses::allResponsesSuccessful() {
    for (const auto[nodeId, response] : this->responses) {
        if(!response.isSuccessful){
            return true;
        }
    }

    return false;
}

MultipleResponsesNotifier::MultipleResponsesNotifier(std::shared_ptr<MultipleResponses> multipleResponse): multipleResponse(multipleResponse) {}

void MultipleResponsesNotifier::addResponse(memdbNodeId_t nodeId, const Response& response) {
    std::unique_lock<std::mutex> uniqueLock(multipleResponse->responsesLock);
    
    multipleResponse->responses.insert({nodeId, response});
    if(response.isSuccessful){
        multipleResponse->successfulResponsesLatch.increase();
    }

    for(const std::function<void(const Response&)>& responseCallback : this->multipleResponse->onResponsesCallbacks) {
        responseCallback(response);
    }
}