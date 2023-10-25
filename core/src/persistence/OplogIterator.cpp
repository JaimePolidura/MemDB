#include "OplogIterator.h"

OplogIterator::OplogIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                             const std::vector<uint8_t>& intermediate,
                             bool compressed,
                             descriptorDataFetcher_t descriptorDataFetcher):
        compressed(compressed),
        descriptorDataFetcher(descriptorDataFetcher),
        intermediate(intermediate),
        descriptors(descriptors) {
}

bool OplogIterator::hasNext() {
    return (this->intermediateIterated && this->actualIndexDescriptor < this->descriptors.size()) ||
        (!this->intermediateIterated && this->intermediate.size() > 0);
}

std::vector<uint8_t> OplogIterator::next() {
    if(!this->intermediateIterated) {
        this->intermediateIterated = true;

        return this->compressed ?
            this->compressor.compressBytes(this->intermediate)
                .get_or_throw_with([](const int errorCode){return "Impossible to compress intermediate. Return code: " + errorCode;}) :
            this->intermediate;
    }

    OplogIndexSegmentDescriptor actualDescriptor = this->descriptors.at(this->actualIndexDescriptor);

    this->actualIndexDescriptor = this->actualIndexDescriptor + 1;
    this->lastTimestampOfLastNext = actualDescriptor.max;
    std::vector<uint8_t> bytesFromDescriptor = this->descriptorDataFetcher(actualDescriptor);

    return !this->compressed ?
           this->compressor.uncompressBytes(bytesFromDescriptor, actualDescriptor.originalSize)
                   .get_or_throw_with([](const int errorCode){return "Impossible to decompress oplog in OplogIterator::next. Error code: " + errorCode;}) :
           bytesFromDescriptor;
}

uint64_t OplogIterator::totalSize() {
    return this->descriptors.size() + (this->intermediate.empty() ? 0 : 1);
}

uint32_t OplogIterator::getNextSize() {
    return this->intermediateIterated ?
        this->descriptors.at(this->actualIndexDescriptor).originalSize :
        this->intermediate.size();
}

bool OplogIterator::isCompressingRequired() {
    return this->compressed;
}

uint32_t OplogIterator::getLastTimestampOfLastNext() {
    return this->lastTimestampOfLastNext;
}