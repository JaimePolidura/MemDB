#include "OplogSegmentIterator.h"

OplogSegmentIterator::OplogSegmentIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                                           const std::vector<OperationBody>& intermediate,
                                           descriptorDataFetcher_t descriptorDataFetcher):
        descriptorDataFetcher(descriptorDataFetcher),
        actualIndexDescriptor(-1),
        intermediate(intermediate),
        descriptors(descriptors) {
}

bool OplogSegmentIterator::hasNext() {
    return this->actualIndexDescriptor >= this->descriptors.size();
}

std::vector<OperationBody> OplogSegmentIterator::next() {
    if(this->actualIndexDescriptor++ == -1){
        return this->intermediate;
    } else {
        return this->descriptorDataFetcher(this->descriptors.at(this->actualIndexDescriptor));
    }
}