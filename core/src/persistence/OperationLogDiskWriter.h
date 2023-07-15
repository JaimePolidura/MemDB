#pragma once

#include <utility>

#include "shared.h"

#include "utils/files/FileUtils.h"
#include "persistence/OperationLogSerializer.h"

class OperationsLogDiskWriter {
private:
    OperationLogSerializer operationLogSerializer;
    std::recursive_mutex writeFileLock;
    bool fileCreated = false;
    bool fileCleared = false;
    std::string oplogFileName;

public:
    OperationsLogDiskWriter() = default;

    explicit OperationsLogDiskWriter(std::string  oplogFileName): oplogFileName(std::move(oplogFileName)) {}

    void clear();

    void write(const std::vector<OperationBody>& toWrite);

    void append(const std::vector<OperationBody>& toWrite);

    void lockWrites();

    void unlockWrites();

private:
    void createFileIfNotExists();

    std::vector<uint8_t> serializeAll(const std::vector<OperationBody>& toSerialize);
};