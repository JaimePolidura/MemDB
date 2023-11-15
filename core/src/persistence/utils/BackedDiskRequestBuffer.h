#pragma once

#include "shared.h"
#include "utils/files/FileUtils.h"
#include "messages/request/Request.h"
#include "utils/Iterator.h"
#include "messages/request/RequestSerializer.h"
#include "messages/request/RequestDeserializer.h"

class BackedDiskBufferIterator : public Iterator<Request> {
    RequestDeserializer deserializer{};

    std::string filePath;
    std::string fileName;
    std::string fullPath;

    std::uintptr_t filePtr;

public:
    BackedDiskBufferIterator(const std::string& filePath, const std::string& fileName, const std::string& fullPath);

    bool hasNext() override;

    Request next() override;

    uint64_t totalSize() override;
};

class BackedDiskRequestBuffer {
    RequestSerializer serializer{};
    std::string filePath;
    std::string fileName;
    std::string fullPath;

public:
    BackedDiskRequestBuffer(const std::string& filePath, const std::string& fileName);

    BackedDiskRequestBuffer() = default;

    void add(const Request& request);

    BackedDiskBufferIterator iterator();
};
