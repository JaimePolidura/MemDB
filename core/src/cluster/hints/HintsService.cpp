#include "HintsService.h"

HintsService::HintsService(configuration_t configuration): configuration(configuration) {}

void HintsService::add(memdbNodeId_t nodeId, const Request&request) {
    std::unique_lock unique_lock(this->hintsServiceLock);

    if(this->hintsByNodeId.contains(nodeId)) {
        this->hintsByNodeId.at(nodeId)->add(request);
        return;
    }

    this->hintsByNodeId.insert({nodeId, std::make_shared<BackedDiskRequestBuffer>(
        this->configuration->get(ConfigurationKeys::DATA_PATH) + "/hints",
        std::to_string(nodeId)
    )});

    this->hintsByNodeId.at(nodeId)->add(request);
}

iterator_t<Request> HintsService::iterator(memdbNodeId_t nodeId) {
    std::unique_lock uniqueLock(this->hintsServiceLock);

    if(this->hintsByNodeId.contains(nodeId) && !this->onGoingIterators.contains(nodeId)){
        iterator_t<Request> bufferIterator = this->hintsByNodeId.at(nodeId)->iterator();
        this->onGoingIterators.insert(nodeId);

        return std::make_shared<HintIterator>(std::dynamic_pointer_cast<BackedDiskBufferIterator>(bufferIterator), [this, nodeId]() -> void {
            std::unique_lock uniqueLock2(this->hintsServiceLock);
            this->onGoingIterators.erase(nodeId);
        });
    } else {
        return std::make_shared<NullIterator<Request>>();
    }
}

void HintsService::clear(memdbNodeId_t nodeId) {
    if(this->hintsByNodeId.contains(nodeId)) {
        this->hintsByNodeId.at(nodeId)->clear();
    }
}
