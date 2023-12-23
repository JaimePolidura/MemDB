#pragma once

#include "memdbtypes.h"

struct ReplicateCounterRequest {
    memdbNodeId_t otherNodeId;
    bool isIncrement;
    int64_t newValue;
    int64_t lastSeenSelfIncrement;
    int64_t lastSeenSelfDecrement;
};
