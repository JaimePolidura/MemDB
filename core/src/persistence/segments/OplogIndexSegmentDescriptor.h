#pragma once

#include "shared.h"

struct OplogIndexSegmentDescriptor {
    uint64_t min;
    uint64_t max;
    uint64_t ptr;
    uint32_t crc;
    uint32_t flags;
};