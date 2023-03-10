#pragma once

#include "shared.h"

#include "messages/request/Request.h"
#include "SingleThreadedLogCompacter.h"

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
    MultiThreadedCompactionBlock(bool firstPhase, std::vector<OperationBody> operations):
            firstPhase(firstPhase), operationsFirstPhase(operations), left(nullptr), right(nullptr) {}

    MultiThreadedCompactionBlock(): firstPhase(false), left(nullptr), right(nullptr) {}

    std::future<std::vector<OperationBody>> get() {
        if(this->firstPhase){
            return this->futureOfOperationsFirstPhase();
        }

        std::future<std::vector<OperationBody>> compactionBlockLeft = this->left->get();
        std::future<std::vector<OperationBody>> compactionBlockRight = this->right->get();

        return this->mergeAndCompactAsync(compactionBlockLeft, compactionBlockRight);
    }

private:
    std::future<std::vector<OperationBody>> mergeAndCompactAsync(std::future<std::vector<OperationBody>>& leftToCompact,
                                                                 std::future<std::vector<OperationBody>>& rightToCompact) {
        return std::async(std::launch::async, [this](std::shared_future<std::vector<OperationBody>> leftToCompactParam,
                                                     std::shared_future<std::vector<OperationBody>> rightToCompactParam){
            std::vector<OperationBody> compacted{};
            alreadySennKeys_t alreadySennKeys{};

            this->operationLogCompacter.compact(rightToCompactParam.get(), compacted, alreadySennKeys);
            this->operationLogCompacter.compact(leftToCompactParam.get(), compacted, alreadySennKeys);

            return compacted;
        }, leftToCompact.share(), rightToCompact.share());
    }

    std::future<std::vector<OperationBody>> futureOfOperationsFirstPhase() {
        std::promise<std::vector<OperationBody>> promise;
        promise.set_value(this->operationsFirstPhase);

        return promise.get_future();
    }

public:
    static MultiThreadedCompactionBlock * node() {
        return new MultiThreadedCompactionBlock();
    }

    static MultiThreadedCompactionBlock * root() {
        return new MultiThreadedCompactionBlock();
    }

    static MultiThreadedCompactionBlock * leaf(allOperationLogs_t uncompacted, int numberBlock, int totalBlocks) {
        int logsPerBlock = uncompacted->size() / totalBlocks;
        auto beginPtr = uncompacted->begin() + (logsPerBlock * numberBlock);
        auto endPtr = numberBlock + 1 != totalBlocks ? uncompacted->begin() + (logsPerBlock * (numberBlock + 1)) : uncompacted->end();

        return new MultiThreadedCompactionBlock(true, std::vector<OperationBody>(beginPtr, endPtr));
    }
};