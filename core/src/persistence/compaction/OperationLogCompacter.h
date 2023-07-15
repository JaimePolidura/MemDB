#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "persistence/compaction/MultithreadedLogCompactor.h"
#include "persistence/compaction/SingleThreadedLogCompacter.h"

class OperationLogCompacter {
private:
    SingleThreadedLogCompacter singleThreaded;
    MultiThreadedLogCompactor multiThreaded;

public:
    std::vector<OperationBody> compact(const std::vector<OperationBody>& uncompacted);
};