#pragma once

#include "shared.h"
#include "utils/clock/LamportClock.h"
#include "memdbtypes.h"

struct OperationOptions {
public:
    bool checkTimestamps;
    bool onlyExecute;
    bool dontBroadcastToCluster;
    bool dontSaveInOperationLog;
    bool dontDebugLog;
    LamportClock::UpdateClockStrategy updateClockStrategy;
    memdbPartitionId_t partitionId;

    uint64_t requestNumber;

    bool fromClient() const {
        return !this->checkTimestamps;
    }
};