#pragma once

#include "shared.h"
#include "utils/clock/LamportClock.h"

struct OperationOptions {
public:
    bool checkTimestamps;
    bool onlyExecute;
    bool dontBroadcastToCluster;
    bool dontSaveInOperationLog;
    LamportClock::UpdateClockStrategy updateClockStrategy;

    uint64_t requestNumber;
    bool lastFragmentMultiResponses;

    bool fromClient() const {
        return !this->checkTimestamps;
    }
};