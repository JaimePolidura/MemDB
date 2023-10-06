#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "MultithreadedCompactionBlock.h"

class MultiThreadedLogCompactor {
public:
    static std::vector<OperationBody> compact(allOperationLogs_t uncompacted);

private:
    static MultiThreadedCompactionBlock * createCompactionTree(allOperationLogs_t uncompacted);

    static std::map<int, std::vector<MultiThreadedCompactionBlock *>> createBlocksByPhaseMap(allOperationLogs_t uncompacted);

    static std::map<int, std::vector<MultiThreadedCompactionBlock *>> fillTreeChilds(std::map<int, std::vector<MultiThreadedCompactionBlock *>>& blocksByPhase);
};