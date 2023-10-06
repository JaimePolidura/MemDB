#include "OplogIndexSegmentDescriptorSerializer.h"

std::vector<uint8_t> OplogIndexSegmentDescriptorSerializer::serialize(const OplogIndexSegmentDescriptor descriptor) {
    std::vector<uint8_t> bytes{};

    Utils::appendToBuffer(descriptor.min, bytes);
    Utils::appendToBuffer(descriptor.max, bytes);
    Utils::appendToBuffer(descriptor.ptr, bytes);
    Utils::appendToBuffer(descriptor.crc, bytes);
    Utils::appendToBuffer(descriptor.flags, bytes);

    return bytes;
}