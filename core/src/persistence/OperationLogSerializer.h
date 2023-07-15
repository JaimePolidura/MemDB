#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "utils/Utils.h"

class OperationLogSerializer {
public:
    OperationLogSerializer() = default;

    std::shared_ptr<std::vector<uint8_t>> serializeAllShared(const std::vector<OperationBody>& toSerialize);

    std::vector<uint8_t> serializeAll(const std::vector<OperationBody>& toSerialize);

    std::vector<uint8_t> serialize(std::vector<uint8_t>& serializedOut, const OperationBody& toDeserialize);

private:
    void writePadding(std::vector<uint8_t>& buffer);
};