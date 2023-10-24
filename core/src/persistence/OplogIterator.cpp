#include "OplogIterator.h"

OplogIterator::OplogIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                             const std::vector<uint8_t>& intermediate,
                             descriptorDataFetcher_t descriptorDataFetcher):
        descriptorDataFetcher(descriptorDataFetcher),
        actualIndexDescriptor(0),
        intermediate(intermediate),
        descriptors(descriptors) {
}

bool OplogIterator::hasNext() {
    return (this->intermediateIterated && this->actualIndexDescriptor < this->descriptors.size()) ||
        (!this->intermediateIterated && this->intermediate.size() > 0);
}

std::vector<uint8_t> OplogIterator::next() {
    if(!this->intermediateIterated){
        this->intermediateIterated = true;
        return this->intermediate;
    }

    OplogIndexSegmentDescriptor actualDescriptor = this->descriptors.at(this->actualIndexDescriptor);

    this->actualIndexDescriptor = this->actualIndexDescriptor + 1;
    this->lastTimestampOfLastNext = actualDescriptor.max;

    return this->descriptorDataFetcher(actualDescriptor);
}

uint64_t OplogIterator::totalSize() {
    return this->descriptors.size() + (this->intermediate.empty() ? 0 : 1);
}

uint32_t OplogIterator::getLastTimestampOfLastNext() {
    return this->lastTimestampOfLastNext;
}