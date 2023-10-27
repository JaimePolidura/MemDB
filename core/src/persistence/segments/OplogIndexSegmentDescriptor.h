#pragma once

#include "shared.h"

enum OplogIndexSegmentDescriptorFlag : uint8_t {
};

struct OplogIndexSegmentDescriptor {
    uint64_t min;
    uint64_t max;
    uint64_t ptr;
    uint32_t crc;
    uint32_t size;
    uint32_t uncompressedSize;
    uint32_t flags;

    bool hasFlag(OplogIndexSegmentDescriptorFlag flag) {
        return (this->flags & flag) == flag;
    }
};