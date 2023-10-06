#pragma once

#include "persistence/segments/OplogIndexSegmentDescriptor.h"
#include "utils/Utils.h"
#include "shared.h"

class OplogIndexSegmentDescriptorSerializer {
public:
    std::vector<uint8_t> serialize(const OplogIndexSegmentDescriptor descriptor);
};
