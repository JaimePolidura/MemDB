#pragma once

#include "shared.h"

#include "persistence/segments/OplogIndexSegmentDescriptor.h"
#include "messages/request/Request.h"

using descriptorDataFetcher_t = std::function<std::vector<OperationBody>(OplogIndexSegmentDescriptor)>;

class OplogSegmentIterator {
private:
    std::vector<OplogIndexSegmentDescriptor> descriptors;
    descriptorDataFetcher_t descriptorDataFetcher;
    std::vector<OperationBody> intermediate;

    int actualIndexDescriptor;

public:
    OplogSegmentIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                         const std::vector<OperationBody>& intermediate,
                         descriptorDataFetcher_t descriptorDataFetcher);

    bool hasNext();

    std::vector<OperationBody> next();
};