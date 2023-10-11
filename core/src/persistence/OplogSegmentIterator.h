#pragma once

#include "shared.h"

#include "persistence/segments/OplogIndexSegmentDescriptor.h"
#include "messages/request/Request.h"
#include "utils/Iterator.h"

using descriptorDataFetcher_t = std::function<std::vector<uint8_t>(OplogIndexSegmentDescriptor)>;

class OplogSegmentIterator : public Iterator<std::vector<uint8_t>> {
private:
    std::vector<OplogIndexSegmentDescriptor> descriptors;
    descriptorDataFetcher_t descriptorDataFetcher;
    std::vector<uint8_t> intermediate;

    int actualIndexDescriptor;

public:
    OplogSegmentIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                         const std::vector<uint8_t>& intermediate,
                         descriptorDataFetcher_t descriptorDataFetcher);

    bool hasNext() override;

    std::vector<uint8_t> next() override;

    uint64_t totalSize() override;
};

using oplogSegmentIterator_t = std::shared_ptr<OplogSegmentIterator>;