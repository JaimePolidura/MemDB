#pragma once

#include "memdbtypes.h"

struct ReplicateCounterRequest {
    memdbNodeId_t otherNodeId;
    bool isIncrement;
    uint64_t newValue;
    uint64_t lastSeenSelfIncrement;
    uint64_t lastSeenSelfDecrement;
};
