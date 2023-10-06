#pragma once

#include "shared.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "persistence/utils/OperationLogDeserializer.h"
#include "persistence/utils/OperationLogSerializer.h"

class OperationLogIntermediateFlusher {
private:
    OperationLogDeserializer deserializer;
    OperationLogSerializer serializer;
    OperationLogCompacter compacter;

public:
    void flush(const std::vector<uint8_t>& toFlush);
};