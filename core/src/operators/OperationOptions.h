#pragma once

#include "shared.h"
#include "utils/clock/LamportClock.h"

struct OperationOptions {
public:
    bool checkTimestamps;
    bool onlyExecute;
    bool dontBroadcastToCluster;
    bool dontSaveInOperationLog;
    bool dontDebugLog;
    LamportClock::UpdateClockStrategy updateClockStrategy;
    uint32_t partitionId;

    uint64_t requestNumber;

    bool fromClient() const {
        return !this->checkTimestamps;
    }
};