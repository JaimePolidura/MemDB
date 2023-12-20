#pragma once

#include "shared.h"
#include "memdbtypes.h"

struct RingEntry {
    memdbNodeId_t nodeId;
    uint32_t ringPosition;
};