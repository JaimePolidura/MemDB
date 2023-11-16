#pragma once

#include "memdbtypes.h"
#include "shared.h"
#include "persistence/utils/BackedDiskRequestBuffer.h"
#include "messages/request/Request.h"

class HintIterator : public Iterator<Request> {
    backedDiskBufferIterator_t diskBufferIterator;
    std::function<void()> onStopIterating;

public:
    HintIterator(backedDiskBufferIterator_t diskBufferIterator, std::function<void()> onStopIterating);

    Request next() override;

    bool hasNext() override;

    uint64_t totalSize() override;
};