#include "OperationLogIntermediateFlusher.h"

void OperationLogIntermediateFlusher::flush(const std::vector<uint8_t> &toFlush) {
    std::vector<OperationBody> operations = this->deserializer.deserializeAll(toFlush);
    std::vector<OperationBody> compacted = this->compacter.compact(operations);
    std::sort(compacted.begin(), compacted.end(), [](const OperationBody& a, const OperationBody& b){return a.timestamp - b.timestamp;});
    std::vector<uint8_t> compactedSerialized = this->serializer.serializeAll(compacted);


}
