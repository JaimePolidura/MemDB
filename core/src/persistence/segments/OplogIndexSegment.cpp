#include "OplogIndexSegment.h"

const std::string OplogIndexSegment::INDEX_FILE_NAME = "oplog-index";
const std::string OplogIndexSegment::DATA_FILE_NAME = "oplog-data";

OplogIndexSegment::OplogIndexSegment(configuration_t configuration, logger_t loggerCons, uint32_t oplogId):
        configuration(configuration),
        logger(loggerCons),
        memdDbBasePath(configuration->get(ConfigurationKeys::DATA_PATH)),
        partitionPath(memdDbBasePath + "/" + std::to_string(oplogId)),
        fullPathIndex(partitionPath + "/" + INDEX_FILE_NAME),
        fullPathData(partitionPath + "/" + DATA_FILE_NAME),
        oplogIndexSegmentReader(fullPathIndex, fullPathData, INDEX_FILE_NAME, partitionPath, logger),
        oplogIndexSegmentWriter(partitionPath, DATA_FILE_NAME, INDEX_FILE_NAME, logger) {
    this->initializeFiles();
}

void OplogIndexSegment::save(const std::vector<uint8_t>& toSave) {
    this->oplogIndexSegmentWriter.write(toSave);
}

OplogSegmentAfterTimestampsSearchResult OplogIndexSegment::getByAfterTimestamp(uint64_t timestamp) {
    OplogSegmentBinarySearchResult searchResult = this->binarySearchByTimestamp(timestamp);
    uint64_t ptrToRead = searchResult.found ? searchResult.ptr : 0;

    return {.descriptors = this->oplogIndexSegmentReader.readAllIndexFrom(ptrToRead), .descriptorInitPtr = ptrToRead};
}

OplogSegmentBetweenTimestampsSearchResult OplogIndexSegment::getBetweenTimestamps(uint64_t fromTimestamp, uint64_t toTimestamp) {
    auto fromResult = this->binarySearchByTimestamp(fromTimestamp);
    auto toResult = this->binarySearchByTimestamp(toTimestamp);

    if(!fromResult.found && !toResult.found){
        return OplogSegmentBetweenTimestampsSearchResult{
                .resultsInDescriptorsAndIntermediate = false,
                .resultsOnlyInIntermediate = true,
                .resultsOnlyInSegments = false,
                .descriptorInitPtr = 0,
                .descriptors = {}
        };
    }
    if(fromResult.found && !toResult.found){
        return OplogSegmentBetweenTimestampsSearchResult{
                .resultsInDescriptorsAndIntermediate = true,
                .resultsOnlyInIntermediate = false,
                .resultsOnlyInSegments = false,
                .descriptorInitPtr = fromResult.ptr,
                .descriptors = oplogIndexSegmentReader.readAllIndexFrom(fromResult.ptr),
        };
    }
    if(toResult.found && fromResult.found){
        return OplogSegmentBetweenTimestampsSearchResult{
                .resultsInDescriptorsAndIntermediate = false,
                .resultsOnlyInIntermediate = false,
                .resultsOnlyInSegments = true,
                .descriptorInitPtr = fromResult.ptr,
                .descriptors = oplogIndexSegmentReader.readAllIndexBetween(fromResult.ptr, toResult.ptr),
        };
    }

    throw std::runtime_error("fromResult and toResult in OplogIndexSegment::getBetweenTimestamps cannot be false");
}

std::vector<OplogIndexSegmentDescriptor> OplogIndexSegment::getAll() {
    return this->oplogIndexSegmentReader.readAllIndex();
}

void OplogIndexSegment::updateCorruptedSegment(const std::vector<uint8_t>& uncorruptedBytes, uint32_t uncompressedSize, uint64_t indexSegmentDescriptorPtr) {
    OplogIndexSegmentDescriptor corruptedDescriptor = this->oplogIndexSegmentReader.readIndexAt(indexSegmentDescriptorPtr);
    this->oplogIndexSegmentWriter.updateCorruptedSegment(uncompressedSize, indexSegmentDescriptorPtr, corruptedDescriptor, uncorruptedBytes);
}

std::result<std::vector<uint8_t>> OplogIndexSegment::getDataByDescriptorBytes(OplogIndexSegmentDescriptor descriptor) {
    return this->oplogIndexSegmentReader.readBytesDataByDescriptor(descriptor);
}

void OplogIndexSegment::clearAll() {
    FileUtils::clear(this->fullPathIndex);
    FileUtils::clear(this->fullPathData);
}

void OplogIndexSegment::initializeFiles() {
    if(!FileUtils::exists(memdDbBasePath)){
        FileUtils::createDirectory(memdDbBasePath);
    }
    if(!FileUtils::exists(partitionPath)){
        FileUtils::createDirectory(partitionPath);
    }
    if(!FileUtils::exists(fullPathIndex)){
        FileUtils::createFile(partitionPath, OplogIndexSegment::INDEX_FILE_NAME);
    }
    if(!FileUtils::exists(fullPathData)){
        FileUtils::createFile(partitionPath, OplogIndexSegment::DATA_FILE_NAME);
    }
}

OplogSegmentBinarySearchResult OplogIndexSegment::binarySearchByTimestamp(uint64_t timestamp) {
    uint64_t indexFileSize = FileUtils::size(partitionPath, INDEX_FILE_NAME);
    if(indexFileSize == 0 || timestamp == 0){
        return OplogSegmentBinarySearchResult{.found = false};
    }

    uint64_t nDescriptors = indexFileSize / sizeof(OplogIndexSegmentDescriptor);
    uint64_t nDescriptorHalf = nDescriptors % 2 == 0 ? nDescriptors / 2 : (nDescriptors / 2) + 1 ;

    uint64_t actual = nDescriptorHalf * sizeof(OplogIndexSegmentDescriptor) - sizeof(OplogIndexSegmentDescriptor);
    uint64_t low = 0;
    uint64_t high = indexFileSize;

    OplogIndexSegmentDescriptor actualDesc = this->oplogIndexSegmentReader.readIndexAt(actual);

    while(high != low && ((timestamp > actualDesc.min && timestamp > actualDesc.max) || (timestamp < actualDesc.min && timestamp < actualDesc.max))){
        bool isHigher = timestamp > actualDesc.min && timestamp > actualDesc.max;

        if(isHigher)
            low = actual;
        else
            high = actual;

        nDescriptors = (high - low) / sizeof(OplogIndexSegmentDescriptor);
        nDescriptorHalf = (low / sizeof(OplogIndexSegmentDescriptor)) + (nDescriptors == 1 ? 0 : (nDescriptors % 2 == 0 ? nDescriptors / 2 : (nDescriptors / 2) + 1));
        actual = nDescriptorHalf * sizeof(OplogIndexSegmentDescriptor);
        actualDesc = this->oplogIndexSegmentReader.readIndexAt(actual);
    }

    return OplogSegmentBinarySearchResult{.found = true, .ptr = actual};
}