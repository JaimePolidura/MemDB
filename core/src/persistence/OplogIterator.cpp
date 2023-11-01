#include "OplogIterator.h"

OplogIterator::OplogIterator(const std::vector<OplogIndexSegmentDescriptor>& descriptors,
                             uint64_t segmentOplogDescriptorInitDiskPtr,
                             const std::vector<uint8_t>& intermediate,
                             bool iterateOnlyDescriptors,
                             bool compressed,
                             uint32_t oplogId,
                             descriptorDataFetcher_t descriptorDataFetcher):
        compressed(compressed),
        descriptorDataFetcher(descriptorDataFetcher),
        intermediateIterated(iterateOnlyDescriptors),
        segmentOplogDescriptorInitDiskPtr(segmentOplogDescriptorInitDiskPtr),
        intermediate(intermediate),
        oplogId(oplogId),
        descriptors(descriptors) {
}

bool OplogIterator::hasNext() {
    return (this->intermediateIterated && this->actualIndexDescriptor < this->descriptors.size()) ||
        (!this->intermediateIterated && this->intermediate.size() > 0);
}

std::result<std::vector<uint8_t>> OplogIterator::next() {
    if(!this->intermediateIterated) {
        this->intermediateIterated = true;

        return this->compressed ?
            std::ok(this->compressor.compressBytes(this->intermediate)
                .get_or_throw_with([](const int errorCode){return "Impossible to compress intermediate. Return code: " + errorCode;})) :
            std::ok(this->intermediate);
    }

    OplogIndexSegmentDescriptor actualDescriptor = this->descriptors.at(this->actualIndexDescriptor);

    this->actualIndexDescriptor = this->actualIndexDescriptor + 1;
    this->lastTimestampOfLastNext = actualDescriptor.max;
    //Maybe corrupted
    std::result<std::vector<uint8_t>> bytesFromDescriptorResult = this->descriptorDataFetcher(actualDescriptor);

    return !this->compressed && bytesFromDescriptorResult.is_success() ?
           std::ok(this->compressor.uncompressBytes(bytesFromDescriptorResult.get(), actualDescriptor.uncompressedSize)
                   .get_or_throw_with([](const int errorCode){return "Impossible to decompress oplog in OplogIterator::next. Error code: " + errorCode;})) :
           bytesFromDescriptorResult;
}

uint64_t OplogIterator::totalSize() {
    return this->descriptors.size() + (this->intermediate.empty() ? 0 : 1);
}

uint64_t OplogIterator::getLastSegmentOplogDescriptorDiskPtr() {
    if(this->actualIndexDescriptor > 0){
        return this->segmentOplogDescriptorInitDiskPtr + ((this->actualIndexDescriptor - 1) * sizeof(OplogIndexSegmentDescriptor));
    } else {
        return this->segmentOplogDescriptorInitDiskPtr;
    }
}

uint32_t OplogIterator::getNextUncompressedSize() {
    return this->intermediateIterated ?
        this->descriptors.at(this->actualIndexDescriptor).uncompressedSize :
        this->intermediate.size();
}

uint32_t OplogIterator::getLastUncompressedSize() {
    if(this->actualIndexDescriptor == 0){
        return this->intermediate.size();
    } else {
        return this->descriptors.at(this->actualIndexDescriptor - 1).uncompressedSize;
    }
}

OplogIndexSegmentDescriptor OplogIterator::getLastDescriptor() {
    return this->descriptors.at(this->actualIndexDescriptor - 1);
}

bool OplogIterator::isCompressingRequired() {
    return this->compressed;
}

uint32_t OplogIterator::getLastTimestampOfLastNext() {
    return this->lastTimestampOfLastNext;
}

uint32_t OplogIterator::getOplogId() {
    return this->oplogId;
}