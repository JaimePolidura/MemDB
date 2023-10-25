#pragma once

#include "shared.h"

#include "persistence/segments/OplogIndexSegmentDescriptor.h"
#include "messages/request/Request.h"
#include "utils/compression/Compressor.h"
#include "utils/Iterator.h"

using descriptorDataFetcher_t = std::function<std::vector<uint8_t>(OplogIndexSegmentDescriptor)>;

class OplogIterator : public Iterator<std::vector<uint8_t>> {
private:
    std::vector<OplogIndexSegmentDescriptor> descriptors;
    descriptorDataFetcher_t descriptorDataFetcher;
    std::vector<uint8_t> intermediate;

    Compressor compressor;

    bool compressed;

    //TODO Not being updated when doing next() on intermediate
    uint32_t lastTimestampOfLastNext;

    uint32_t actualIndexDescriptor;
    bool intermediateIterated;

public:
    OplogIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                  const std::vector<uint8_t>& intermediate,
                  bool compressed,
                  descriptorDataFetcher_t descriptorDataFetcher);

    bool hasNext() override;

    std::vector<uint8_t> next() override;

    uint64_t totalSize() override;

    uint32_t getActualSize();

    uint32_t getNextSize();

    uint32_t getLastTimestampOfLastNext();

    bool isCompressingRequired();
};

using oplogSegmentIterator_t = std::shared_ptr<OplogIterator>;