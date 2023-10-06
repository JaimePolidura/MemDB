#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "persistence/compaction/singlethreaded/SingleThreadedLogCompacter.h"

using allOperationLogs_t = std::shared_ptr<std::vector<OperationBody>>;

class MultiThreadedCompactionBlock {
public:
    MultiThreadedCompactionBlock * left;
    MultiThreadedCompactionBlock * right;
private:
    std::vector<OperationBody> operationsFirstPhase;
    SingleThreadedLogCompacter operationLogCompacter;
    bool firstPhase;

public:
    MultiThreadedCompactionBlock(bool firstPhase, std::vector<OperationBody> operations);

    MultiThreadedCompactionBlock();

    std::future<std::vector<OperationBody>> get();

private:
    std::future<std::vector<OperationBody>> mergeAndCompactAsync(std::future<std::vector<OperationBody>>& leftToCompact,
                                                                 std::future<std::vector<OperationBody>>& rightToCompact);

    std::future<std::vector<OperationBody>> futureOfOperationsFirstPhase();

public:
    static MultiThreadedCompactionBlock * node();

    static MultiThreadedCompactionBlock * root();

    static MultiThreadedCompactionBlock * leaf(allOperationLogs_t uncompacted, int numberBlock, int totalBlocks);
};