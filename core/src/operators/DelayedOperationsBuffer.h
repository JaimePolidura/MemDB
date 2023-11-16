#pragma once

#include "persistence/utils/BackedDiskRequestBuffer.h"
#include "messages/request/Request.h"
#include "shared.h"
#include "config/Configuration.h"

class DelayedOperationsBuffer {
    backedDiskRequestBuffer_t diskBuffer;

public:
    explicit DelayedOperationsBuffer(configuration_t configuration);

    DelayedOperationsBuffer() = default;

    void add(const Request& request);

    iterator_t<Request> iterator();
};
