#pragma once

#include "shared.h"

struct ReplicateCounterResponse {
    bool needsIncrementSync{false};
    bool needsDecrementSync{false};
    uint64_t nIncrementToSync{0};
    uint64_t nDecrementToSync{0};
};