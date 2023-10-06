#pragma once

#include <utility>

#include "utils/files/FileUtils.h"
#include "utils/datastructures/map/Map.h"
#include "utils/Utils.h"

#include "persistence/serializers/OperationLogDeserializer.h"
#include "operators/operations/user/SetOperator.h"
#include "persistence/utils/OperationLogSerializer.h"
#include "config/Configuration.h"

class OperationLogDiskLoader {
private:
    OperationLogDeserializer operationLogDeserializer{};
    OperationLogSerializer operationLogSerializer;
    std::string oplogFileName;
    configuration_t configuration;

public:
    OperationLogDiskLoader() = default;

    explicit OperationLogDiskLoader(std::string oplogFileName, configuration_t configuration): oplogFileName(std::move(oplogFileName)), configuration(configuration) {}

    std::vector<OperationBody> getAll();

private:
    void writeToDisk(const std::vector<OperationBody>& toWrite);
};