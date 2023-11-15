#pragma once

#include "persistence/utils/BackedDiskRequestBuffer.h"
#include "messages/request/Request.h"
#include "shared.h"
#include "config/Configuration.h"

class DelayedOperationsBuffer {
    BackedDiskRequestBuffer diskBuffer;

public:
    explicit DelayedOperationsBuffer(configuration_t configuration);

    DelayedOperationsBuffer() = default;

    void add(const Request& request);

    BackedDiskBufferIterator iterator();
};
