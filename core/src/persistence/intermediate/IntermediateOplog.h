#pragma once

#include "shared.h"
#include "messages/request/Request.h"
#include "config/Configuration.h"
#include "utils/files/FileUtils.h"
#include "persistence/serializers/OperationLogSerializer.h"
#include "persistence/serializers/OperationLogDeserializer.h"

class IntermediateOplog {
private:
    OperationLogDeserializer operationLogDeserializer;
    OperationLogSerializer operationLogSerializer;

    configuration_t configuration;
    uint64_t bytesWritten;

    std::function<void(const std::vector<uint8_t>&)> onFlushingIntermediate;

    const std::string memdDbBasePath;
    const std::string partitionPath;
    const std::string fullPath;

public:
    static const std::string FILE_NAME;

    IntermediateOplog(configuration_t configuration,
                      uint32_t oplogId);

    void add(const OperationBody& operation);

    void addAll(const std::vector<OperationBody>& operations);

    void setOnFlushingIntermediate(const std::function<void(const std::vector<uint8_t>&)> callback);

    std::vector<uint8_t> getAllBytes();

    void clearAll();

private:
    void initializeFiles();
};

using intermediateOplog_t = std::shared_ptr<IntermediateOplog>;