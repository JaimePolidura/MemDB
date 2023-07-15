#pragma once

#include <utility>

#include "utils/files/FileUtils.h"
#include "utils/datastructures/map/Map.h"
#include "utils/Utils.h"

#include "persistence/OperationLogDeserializer.h"
#include "operators/operations/user/SetOperator.h"
#include "persistence/OperationLogSerializer.h"

class OperationLogDiskLoader {
private:
    OperationLogDeserializer operationLogDeserializer{};
    OperationLogSerializer operationLogSerializer;
    std::string oplogFileName;

public:
    OperationLogDiskLoader() = default;

    explicit OperationLogDiskLoader(std::string oplogFileName): oplogFileName(std::move(oplogFileName)) {}

    std::vector<OperationBody> getAll();

private:
    void writeToDisk(const std::vector<OperationBody>& toWrite);
};