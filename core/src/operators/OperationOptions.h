#pragma once

#include "shared.h"

struct OperationOptions {
    bool checkTimestamps;
    bool onlyExecute;
    bool dontBroadcastToCluster;
    bool dontSaveInOperationLog;
};