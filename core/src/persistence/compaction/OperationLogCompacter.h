#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "persistence/compaction/MultiThreadedLogCompactor.h"
#include "persistence/compaction/SingleThreadedLogCompacter.h"

class OperationLogCompacter {
private:
    MultiThreadedLogCompactor multiThreaded;
    SingleThreadedLogCompacter singleThreaded;

public:
    std::vector<OperationBody> compact(const std::vector<OperationBody>& uncompacted) {
        return uncompacted.size() > 10.000 ?
            this->multiThreaded.compact(std::make_shared<std::vector<OperationBody>>(std::move(uncompacted))) :
            this->singleThreaded.compact(uncompacted);
    }
};