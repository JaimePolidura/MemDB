#include "BackedDiskRequestBuffer.h"

BackedDiskRequestBuffer::BackedDiskRequestBuffer(const std::string& filePath, const std::string& fileName): fileName(fileName), filePath(filePath), fullPath(filePath + "/" + fileName) {
    if(!FileUtils::exists(fullPath)) {
        FileUtils::createFile(filePath, fileName);
    }
}


void BackedDiskRequestBuffer::add(const Request& request) {
    std::vector<uint8_t> serialized = this->serializer.serialize(request);
    Utils::appendToBuffer(static_cast<memDbDataLength_t>(serialized.size()), serialized);

    FileUtils::appendBytes(fullPath, serialized);
}

BackedDiskBufferIterator BackedDiskRequestBuffer::iterator() {
    return BackedDiskBufferIterator{this->filePath, this->fileName, this->fullPath};
}

BackedDiskBufferIterator::BackedDiskBufferIterator(const std::string&filePath, const std::string&fileName, const std::string&fullPath):
    filePath(filePath), fileName(fileName), fullPath(fullPath), filePtr(FileUtils::size(filePath, fileName)) {}

Request BackedDiskBufferIterator::next() {
    this->filePtr = this->filePtr - sizeof(memDbDataLength_t);

    std::vector<uint8_t> requestLengthBuffer = FileUtils::seekBytes(fullPath, filePtr, sizeof(memDbDataLength_t));
    memDbDataLength_t requestLength = Utils::parseFromBuffer<memDbDataLength_t>(requestLengthBuffer);

    this->filePtr = this->filePtr - requestLength;

    std::vector<uint8_t> requestBuffer = FileUtils::seekBytes(filePath, filePtr, requestLength);
    Request request = this->deserializer.deserialize(requestBuffer);

    FileUtils::deleteTopBytes(fullPath, requestBuffer.size() + sizeof(memDbDataLength_t));

    return request;
}

bool BackedDiskBufferIterator::hasNext() {
    return this->filePtr > sizeof(memDbDataLength_t);
}


uint64_t BackedDiskBufferIterator::totalSize() {
    throw std::runtime_error("BackedDiskBufferIterator::totalSize not implemented");
}


