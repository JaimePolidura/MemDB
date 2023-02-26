#pragma once

#include <vector>

#include "messages/request/RequestDeserializer.h"

class OperationLogDeserializer {
private:
    RequestDeserializer requestDeserializer;

public:
    std::vector<OperationBody> deserializeAll(const std::vector<uint8_t>& bytes) {
        std::vector<OperationBody> logs{};

        const uint8_t * firstElementPtr = bytes.data();
        const uint8_t * lastElementPtr = &bytes.back();

        for(const uint8_t * bytesPtr = firstElementPtr; bytesPtr < (lastElementPtr + 1);) {
            uint64_t initialOffset = bytesPtr - firstElementPtr;
            OperationBody operationDeserialized = this->requestDeserializer.deserializeOperation(bytes, initialOffset);

            logs.push_back(operationDeserialized);

            uint8_t toIncrease = this->calculateOperationLogSizeInDisk(operationDeserialized);
            bytesPtr = bytesPtr + toIncrease;
        }

        return logs;
    }

private:
    uint8_t calculateOperationLogSizeInDisk(const OperationBody& deserialized) const {
        int totalArgLength = std::accumulate(
                deserialized.args->begin(),
                deserialized.args->end(),
                deserialized.args->size(),
                [](int acc, const auto& it) {
                    return acc + it.size;
                });

        return
            1 + //Operation number
            8 + //Timestamp
            totalArgLength + //Args length (value + size)
            1; //Padding;
    }
};