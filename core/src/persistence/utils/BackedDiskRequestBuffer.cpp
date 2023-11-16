#include "BackedDiskRequestBuffer.h"

BackedDiskRequestBuffer::BackedDiskRequestBuffer(const std::string& filePath, const std::string& fileName):
    fileName(fileName), filePath(filePath), fullPath(filePath + "/" + fileName) {

    if(!FileUtils::exists(fullPath)) {
        FileUtils::createFile(filePath, fileName);
    }
}

void BackedDiskRequestBuffer::add(const Request& request) {
    std::unique_lock uniqueLock(this->lock);

    std::vector<uint8_t> serialized = this->serializer.serialize(request);
    Utils::appendToBuffer(static_cast<memDbDataLength_t>(serialized.size()), serialized);

    FileUtils::appendBytes(fullPath, serialized);
}

void BackedDiskRequestBuffer::clear() {
    std::unique_lock uniqueLock(this->lock);

    FileUtils::clear(this->fullPath);
}

iterator_t<Request> BackedDiskRequestBuffer::iterator() {
    return std::make_shared<BackedDiskBufferIterator>(
        this->filePath, this->fileName, this->fullPath, this->shared_from_this());
}

BackedDiskBufferIterator::BackedDiskBufferIterator(const std::string&filePath, const std::string&fileName, const std::string&fullPath,
    backedDiskRequestBuffer_t backedDiskRequestBufferParent): filePath(filePath), fileName(fileName), fullPath(fullPath),
    uniqueLock(backedDiskRequestBufferParent->lock) {}

Request BackedDiskBufferIterator::next() {
    std::intptr_t filePtr = FileUtils::size(this->filePath, this->fileName);

    filePtr = filePtr - sizeof(memDbDataLength_t);

    std::vector<uint8_t> requestLengthBuffer = FileUtils::seekBytes(fullPath, filePtr, sizeof(memDbDataLength_t));
    memDbDataLength_t requestLength = Utils::parseFromBuffer<memDbDataLength_t>(requestLengthBuffer);

    std::vector<uint8_t> requestBuffer = FileUtils::seekBytes(filePath, filePtr, requestLength);
    Request request = this->deserializer.deserialize(requestBuffer);

    FileUtils::deleteTopBytes(fullPath, requestBuffer.size() + sizeof(memDbDataLength_t));

    return request;
}

bool BackedDiskBufferIterator::hasNext() {
    return FileUtils::size(this->filePath, this->fileName) > sizeof(memDbDataLength_t);
}


uint64_t BackedDiskBufferIterator::totalSize() {
    throw std::runtime_error("BackedDiskBufferIterator::totalSize not implemented");
}


