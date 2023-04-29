#pragma once

#include "persistence/oplog/OperationLog.h"
#include "persistence/oplog/SingleOperationLog.h"

class MultipleOperationLog : public OperationLog {
private:
    std::vector<singleOperationLog_t> operationLogs;

    std::function<int(const OperationBody&)> oplogResolver;

public:
    MultipleOperationLog(configuration_t configuration, std::function<int(const OperationBody&)> oplogResolver,
                         std::function<std::string(int)> oplogFileNameResolver, int numberOplogs):
        OperationLog(configuration), oplogResolver(oplogResolver) {
        this->initializeOplogs(numberOplogs, oplogFileNameResolver);
    }

    void add(const OperationBody& operation) override {
        int oplogId = this->oplogResolver(operation);
        singleOperationLog_t oplog = this->operationLogs[oplogId];

        oplog->add(operation);
    }

    std::vector<OperationBody> getAfterTimestamp(uint64_t timestamp, OperationLogQueryOptions options) override {
        int oplogId = options.operationLogId;
        singleOperationLog_t oplog = this->operationLogs[oplogId];

        return oplog->getAfterTimestamp(timestamp, options);
    }

    std::vector<OperationBody> getAllFromDisk(OperationLogQueryOptions options = {}) override {
        std::vector<OperationBody> totalFromDisk{};

        for(singleOperationLog_t singleOperationLog : this->operationLogs) {
            std::vector<OperationBody> fromDisk = singleOperationLog->getAllFromDisk();

            totalFromDisk = Utils::concat(totalFromDisk, fromDisk);
        }

        return totalFromDisk;
    }

private:
    void initializeOplogs(int numerOplogs, std::function<std::string(int)> oplogFileNameResolver) {
        for(int i = 0; i < numerOplogs; i++) {
            std::string fileName = oplogFileNameResolver(i);
            singleOperationLog_t oplog = std::make_shared<SingleOperationLog>(this->configuration, fileName);

            this->operationLogs.push_back(oplog);
        }
    }
};
