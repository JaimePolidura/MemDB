#pragma once

#include "shared.h"
#include "utils/clock/LamportClock.h"
#include "memdbtypes.h"

struct OperationOptions {
public:
    bool checkTimestamps{false};
    bool onlyExecute{false};
    bool dontBroadcastToCluster{false};
    bool dontSaveInOperationLog{false};
    bool dontDebugLog{false};
    LamportClock::UpdateClockStrategy updateClockStrategy;
    memdbPartitionId_t partitionId{0};

    uint64_t requestNumber{0};

    bool fromClient() const {
        return !this->checkTimestamps;
    }
};