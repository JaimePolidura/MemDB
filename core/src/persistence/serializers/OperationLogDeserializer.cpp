#include "OperationLogDeserializer.h"

std::vector<OperationBody> OperationLogDeserializer::deserializeAll(const std::vector<uint8_t>& bytes) {
    std::vector<OperationBody> logs{};

    const uint8_t * firstElementPtr = bytes.data();
    const uint8_t * lastElementPtr = &bytes.back();

    for(const uint8_t * bytesPtr = firstElementPtr; bytesPtr < (lastElementPtr + 1);) {
        uint64_t initialOffset = bytesPtr - firstElementPtr;
        OperationBody operationDeserialized = this->requestDeserializer.deserializeOperation(bytes, initialOffset);

        logs.push_back(operationDeserialized);

        uint8_t toIncrease = operationDeserialized.getTotalLength(false) +
                             sizeof(memDbDataLength_t); //Padding
        bytesPtr = bytesPtr + toIncrease;
    }

    return logs;
}