#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "persistence/compaction/multithreaded/MultithreadedLogCompactor.h"
#include "persistence/compaction/singlethreaded/SingleThreadedLogCompacter.h"

class OperationLogCompacter {
private:
    SingleThreadedLogCompacter singleThreaded;
    MultiThreadedLogCompactor multiThreaded;

public:
    std::vector<OperationBody> compact(const std::vector<OperationBody>& uncompacted);
};