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
    std::atomic_bool fileCleared;
    std::string oplogFileName;
    bool fileCreated;

public:
    OperationsLogDiskWriter() = default;

    explicit OperationsLogDiskWriter(std::string oplogFileName, configuration_t configuration):
        oplogFileName(std::move(oplogFileName)),
        configuration(configuration),
        fileCleared(false),
        fileCreated(false) {}

    void clear();

    void append(const std::vector<OperationBody>& toWrite);

private:
    void createFileIfNotExists();

    std::vector<uint8_t> serializeAll(const std::vector<OperationBody>& toSerialize);
};