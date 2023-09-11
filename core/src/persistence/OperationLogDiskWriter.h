#pragma once

#include <utility>

#include "shared.h"

#include "utils/files/FileUtils.h"
#include "persistence/OperationLogSerializer.h"
#include "config/Configuration.h"

class OperationsLogDiskWriter {
private:
    OperationLogSerializer operationLogSerializer;
    configuration_t configuration;
    std::recursive_mutex writeFileLock;
    bool fileCreated = false;
    bool fileCleared = false;
    std::string oplogFileName;

public:
    OperationsLogDiskWriter() = default;

    explicit OperationsLogDiskWriter(std::string oplogFileName, configuration_t configuration): oplogFileName(std::move(oplogFileName)), configuration(configuration) {}

    void clear();

    void write(const std::vector<OperationBody>& toWrite);

    void append(const std::vector<OperationBody>& toWrite);

    void lockWrites();

    void unlockWrites();

private:
    void createFileIfNotExists();

    std::vector<uint8_t> serializeAll(const std::vector<OperationBody>& toSerialize);
};