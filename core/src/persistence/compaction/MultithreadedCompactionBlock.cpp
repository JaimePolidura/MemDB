#include "persistence/compaction/MultithreadedCompactionBlock.h"

MultiThreadedCompactionBlock::MultiThreadedCompactionBlock(bool firstPhase, std::vector<OperationBody> operations):
        firstPhase(firstPhase), operationsFirstPhase(operations), left(nullptr), right(nullptr) {}

MultiThreadedCompactionBlock::MultiThreadedCompactionBlock(): firstPhase(false), left(nullptr), right(nullptr) {}

std::future<std::vector<OperationBody>> MultiThreadedCompactionBlock::get() {
    if(this->firstPhase){
        return this->futureOfOperationsFirstPhase();
    }

    std::future<std::vector<OperationBody>> compactionBlockLeft = this->left->get();
    std::future<std::vector<OperationBody>> compactionBlockRight = this->right->get();

    return this->mergeAndCompactAsync(compactionBlockLeft, compactionBlockRight);
}

std::future<std::vector<OperationBody>> MultiThreadedCompactionBlock::mergeAndCompactAsync(
            std::future<std::vector<OperationBody>>& leftToCompact,
            std::future<std::vector<OperationBody>>& rightToCompact) {
    return std::async(std::launch::async, [this](std::shared_future<std::vector<OperationBody>> leftToCompactParam,
                                                 std::shared_future<std::vector<OperationBody>> rightToCompactParam){
        std::vector<OperationBody> compacted{};
        timestampsByKeysMap_t timestampsByKeys{};

        this->operationLogCompacter.compact(rightToCompactParam.get(), compacted, timestampsByKeys);
        this->operationLogCompacter.compact(leftToCompactParam.get(), compacted, timestampsByKeys);

        return compacted;
    }, leftToCompact.share(), rightToCompact.share());
}

std::future<std::vector<OperationBody>> MultiThreadedCompactionBlock::futureOfOperationsFirstPhase() {
    std::promise<std::vector<OperationBody>> promise;
    promise.set_value(this->operationsFirstPhase);

    return promise.get_future();
}

MultiThreadedCompactionBlock * MultiThreadedCompactionBlock::node() {
    return new MultiThreadedCompactionBlock();
}

MultiThreadedCompactionBlock * MultiThreadedCompactionBlock::root() {
    return new MultiThreadedCompactionBlock();
}

MultiThreadedCompactionBlock * MultiThreadedCompactionBlock::leaf(allOperationLogs_t uncompacted, int numberBlock, int totalBlocks) {
    int logsPerBlock = uncompacted->size() / totalBlocks;
    auto beginPtr = uncompacted->begin() + (logsPerBlock * numberBlock);
    auto endPtr = numberBlock + 1 != totalBlocks ? uncompacted->begin() + (logsPerBlock * (numberBlock + 1)) : uncompacted->end();

    return new MultiThreadedCompactionBlock(true, std::vector<OperationBody>(beginPtr, endPtr));
}