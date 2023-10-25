#include "OplogIndexSegmentDescriptorDeserializer.h"

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegmentDescriptorDeserializer::deserializeAll(const std::vector<uint8_t> &bytes) {
    std::vector<OplogIndexSegmentDescriptor> deserialized{bytes.size() / sizeof(OplogIndexSegmentDescriptor)};
    uint8_t * actualPtr = (uint8_t *) bytes.data();

    for(int i = 0; i < bytes.size() / sizeof(OplogIndexSegmentDescriptor); i++){
        deserialized[i] = this->deserializeFromPtr(actualPtr);
        actualPtr += sizeof(OplogIndexSegmentDescriptor);
    }

    return deserialized;
}

OplogIndexSegmentDescriptor OplogIndexSegmentDescriptorDeserializer::deserialize(const std::vector<uint8_t> &bytes) {
    return this->deserializeFromPtr((uint8_t *) bytes.data());
}

OplogIndexSegmentDescriptor OplogIndexSegmentDescriptorDeserializer::deserializeFromPtr(uint8_t * ptr) {
    return OplogIndexSegmentDescriptor{
            .min = Utils::parse<uint64_t>(ptr),
            .max = Utils::parse<uint64_t>(ptr + 8),
            .ptr = Utils::parse<uint64_t>(ptr + 16),
            .crc = Utils::parse<uint32_t>(ptr + 24),
            .size = Utils::parse<uint32_t>(ptr + 28),
            .originalSize = Utils::parse<uint32_t>(ptr + 32),
            .flags = Utils::parse<uint32_t>(ptr + 36),
    };
}