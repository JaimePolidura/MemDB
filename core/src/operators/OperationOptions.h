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

    bool fromClient() const {
        return !this->checkTimestamps;
    }
};