#pragma once

#include "memdbtypes.h"
#include "shared.h"
#include "persistence/utils/BackedDiskRequestBuffer.h"
#include "messages/request/Request.h"
#include "config/Configuration.h"
#include "cluster/hints/HintIterator.h"

class HintsService {
    std::map<memdbNodeId_t, backedDiskRequestBuffer_t> hintsByNodeId{};
    std::set<memdbNodeId_t> onGoingIterators; //We dont want two iterators at the same time
    std::set<memdbNodeId_t> pendingHints{};
    std::set<memdbNodeId_t> hintsNodesCheckedAtLeastOnce{};

    std::mutex hintsServiceLock;

    configuration_t configuration;

public:
    explicit HintsService(configuration_t configuration);

    void add(memdbNodeId_t nodeId, const Request& request);

    bool maybeHasPendingHints(memdbNodeId_t nodeId);

    iterator_t<Request> iterator(memdbNodeId_t nodeId);

    void clear(memdbNodeId_t nodeId);
};

using hintsService_t = std::shared_ptr<HintsService>;