#pragma once

#include <vector>
#include <thread>

#include "messages/request/Request.h"
#include "persistence/OperationLogCompacter.h"
#include "utils/threads/Exchanger.h"

class CompactionBlock {
private:
    OperationLogCompacter operationLogCompacter;

    int phase;
    CompactionBlock * left;
    CompactionBlock * right;
    std::thread compacterThread;
    std::vector<OperationBody> operations;
    std::shared_ptr<std::vector<OperationBody>> sourceData;
    int numberBlock;
    int totalBlocks;
public:
    CompactionBlock(int phase, CompactionBlock * left, CompactionBlock * right, std::shared_ptr<std::vector<OperationBody>> sourceData,
                    int numberBlock, int totalBlocks, std::vector<OperationBody> operations): phase(phase), left(left), right(right), sourceData(sourceData),
                    numberBlock(numberBlock), totalBlocks(totalBlocks), operations(operations) {
    }

    std::vector<OperationBody> get() {
        if(this->firstPhase())
            return this->operations;

        auto compactionBlockLeft = this->left->get();
        auto compactionBlockRight = this->right->get();

        return this->mergeAndCompact(compactionBlockLeft, compactionBlockRight);
    }
    
private:
    std::vector<OperationBody> mergeAndCompact(const std::vector<OperationBody>& leftToCompact, const std::vector<OperationBody>& rightToCompact) {
        std::shared_ptr<Exchanger<std::vector<OperationBody>>> operationsExchanger = std::make_shared<Exchanger<std::vector<OperationBody>>>();

        std::thread thread([this, leftToCompact, rightToCompact, operationsExchanger]{
            std::vector<OperationBody> compacted{};
            std::set<SimpleString<defaultMemDbLength_t>> seenOperationKeys;

            this->compact(rightToCompact, compacted, seenOperationKeys);
            this->compact(leftToCompact, compacted, seenOperationKeys);

            operationsExchanger->asyncEnqueue(compacted);
        });

        return operationsExchanger->dequeue();
    }

    std::vector<OperationBody> compact(const std::vector<OperationBody>& uncompacted,
                                       std::vector<OperationBody>& compactedOut,
                                       std::set<SimpleString<defaultMemDbLength_t>>& seenOperationKeys) {
        for(int i = uncompacted.size() - 1; i >= 0; i--) {
            auto actualOperation = uncompacted.at(i);
            auto actualOperationKey = actualOperation.args->at(0);

            if(seenOperationKeys.contains(actualOperationKey))
                continue;

            seenOperationKeys.insert(actualOperationKey);
            compactedOut.push_back(actualOperation);
        }

        return compactedOut;
    }

    bool lastBlock() {
        return this->numberBlock == this->totalBlocks;
    }

    bool firstPhase() {
        return this->phase == 0;
    }
};