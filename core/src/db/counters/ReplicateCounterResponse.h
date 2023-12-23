#pragma once

#include "shared.h"

struct ReplicateCounterResponse {
    bool needsIncrementSync{false};
    bool needsDecrementSync{false};
    int64_t nIncrementToSync{0};
    int64_t nDecrementToSync{0};
};