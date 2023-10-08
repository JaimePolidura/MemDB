#pragma once

#include "persistence/segments/OplogIndexSegmentDescriptor.h"
#include "utils/Utils.h"
#include "shared.h"

class OplogIndexSegmentDescriptorDeserializer {
public:
    OplogIndexSegmentDescriptor deserialize(const std::vector<uint8_t>& bytes);

    std::vector<OplogIndexSegmentDescriptor> deserializeAll(const std::vector<uint8_t>& bytes);

private:
    OplogIndexSegmentDescriptor deserializeFromPtr(uint8_t * ptr);
};