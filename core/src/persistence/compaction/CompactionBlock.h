#pragma once

#include <vector>
#include <thread>

#include "messages/request/Request.h"
#include "persistence/OperationLogCompacter.h"
#include "utils/threads/Exchanger.h"

using allOperationLogs_t = std::shared_ptr<std::vector<OperationBody>>;

class CompactionBlock {
public:
    CompactionBlock * left;
    CompactionBlock * right;
private:
    std::vector<OperationBody> operationsFirstPhase;
    OperationLogCompacter operationLogCompacter;
    bool firstPhase;

public:
    CompactionBlock(bool firstPhase, std::vector<OperationBody> operations):
            firstPhase(firstPhase), operationsFirstPhase(operations) {}

    CompactionBlock(): firstPhase(false) {}

    std::vector<OperationBody> get() {
        if(this->firstPhase)
            return this->operationsFirstPhase;

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

public:
    static CompactionBlock * node() {
        return new CompactionBlock();
    }

    static CompactionBlock * root() {
        return new CompactionBlock();
    }

    static CompactionBlock * leaf(allOperationLogs_t uncompacted, int numerBlock, int totalBlocks) {
        int logsPerBlock = uncompacted->size() / totalBlocks;
        auto beginPtr = uncompacted->begin() + (logsPerBlock * numerBlock);
        auto endPtr = numerBlock + 1 != totalBlocks ? uncompacted->begin() + (logsPerBlock * (numerBlock + 1)) : uncompacted->end();

        return new CompactionBlock(true, std::vector<OperationBody>(beginPtr, endPtr));
    }
};