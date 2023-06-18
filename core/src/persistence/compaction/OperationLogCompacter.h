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
    std::vector<OperationBody> compact(const std::vector<OperationBody>& uncompacted) {
        return uncompacted.size() > 10.000 ?
            this->multiThreaded.compact(std::make_shared<std::vector<OperationBody>>(std::move(uncompacted))) :
            this->singleThreaded.compact(uncompacted);
    }
};