#pragma once

#include "shared.h"

#include "persistence/segments/OplogIndexSegmentDescriptor.h"
#include "messages/request/Request.h"
#include "utils/compression/Compressor.h"
#include "utils/Iterator.h"

using descriptorDataFetcher_t = std::function<std::result<std::vector<uint8_t>>(OplogIndexSegmentDescriptor)>;

class OplogIterator : public Iterator<std::result<std::vector<uint8_t>>> {
private:
    std::vector<OplogIndexSegmentDescriptor> descriptors;
    descriptorDataFetcher_t descriptorDataFetcher;
    std::vector<uint8_t> intermediate;
    uint32_t oplogId;

    uint64_t segmentOplogDescriptorInitDiskPtr;

    Compressor compressor;

    bool compressed;

    //TODO Not being updated when doing next() on intermediate
    uint32_t lastTimestampOfLastNext;

    uint32_t actualIndexDescriptor{0};
    bool intermediateIterated{false};

public:
    OplogIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                  uint64_t segmentOplogDescriptorInitDiskPtr,
                  const std::vector<uint8_t>& intermediate,
                  bool iterateOnlyDescriptors,
                  bool compressed,
                  uint32_t oplogId,
                  descriptorDataFetcher_t descriptorDataFetcher);

    bool hasNext() override;

    std::result<std::vector<uint8_t>> next() override;

    uint64_t totalSize() override;

    OplogIndexSegmentDescriptor getLastDescriptor();

    uint32_t getOplogId();

    uint64_t getLastSegmentOplogDescriptorDiskPtr();

    uint32_t getNextUncompressedSize();

    uint32_t getLastUncompressedSize();

    uint32_t getLastTimestampOfLastNext();

    bool isCompressingRequired();
};

using oplogIterator_t = std::shared_ptr<OplogIterator>;