#include "OplogIterator.h"

OplogIterator::OplogIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                             const std::vector<uint8_t>& intermediate,
                             descriptorDataFetcher_t descriptorDataFetcher):
        descriptorDataFetcher(descriptorDataFetcher),
        actualIndexDescriptor(-1),
        intermediate(intermediate),
        descriptors(descriptors) {
}

bool OplogIterator::hasNext() {
    return (this->intermediateIterated && this->actualIndexDescriptor + 1 < this->descriptors.size()) ||
            (!this->intermediateIterated && this->intermediate.size() > 0);
}

std::vector<uint8_t> OplogIterator::next() {
    if(this->actualIndexDescriptor++ == -1){
        this->intermediateIterated = true;
        return this->intermediate;
    } else {
        return this->descriptorDataFetcher(this->descriptors.at(this->actualIndexDescriptor));
    }
}

uint64_t OplogIterator::totalSize() {
    return this->descriptors.size() + 1;
}