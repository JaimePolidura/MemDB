#pragma once

#include "shared.h"
#include "utils/files/FileUtils.h"
#include "messages/request/Request.h"
#include "utils/Iterator.h"
#include "messages/request/RequestSerializer.h"
#include "messages/request/RequestDeserializer.h"

class BackedDiskRequestBuffer : public std::enable_shared_from_this<BackedDiskRequestBuffer> {
    RequestSerializer serializer{};
    std::string filePath;
    std::string fileName;
    std::string fullPath;

    std::mutex lock;

    friend class BackedDiskBufferIterator;

public:
    BackedDiskRequestBuffer(const std::string& filePath, const std::string& fileName);

    BackedDiskRequestBuffer() = default;

    void add(const Request& request);

    void clear();

    iterator_t<Request> iterator();
};

using backedDiskRequestBuffer_t = std::shared_ptr<BackedDiskRequestBuffer>;

class BackedDiskBufferIterator : public Iterator<Request> {
    std::unique_lock<std::mutex> uniqueLock;

    RequestDeserializer deserializer{};

    std::string filePath;
    std::string fileName;
    std::string fullPath;

public:
    BackedDiskBufferIterator(const std::string& filePath, const std::string& fileName, const std::string& fullPath,
        backedDiskRequestBuffer_t backedDiskRequestBufferParent);

    bool hasNext() override;

    Request next() override;

    uint64_t totalSize() override;
};

using backedDiskBufferIterator_t = std::shared_ptr<BackedDiskBufferIterator>;

