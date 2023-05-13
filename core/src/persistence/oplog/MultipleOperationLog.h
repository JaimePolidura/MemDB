#pragma once

#include "persistence/oplog/OperationLog.h"
#include "persistence/oplog/SingleOperationLog.h"

//TODO Race conditions
class MultipleOperationLog : public OperationLog {
private:
    std::vector<singleOperationLog_t> operationLogs;

    std::function<int(const OperationBody&)> oplogResolver;

public:
    MultipleOperationLog(configuration_t configuration, std::function<int(const OperationBody&)> oplogResolver,
                         std::function<std::string(int)> oplogFileNameResolver, uint32_t numberOplogs):
        OperationLog(configuration), oplogResolver(oplogResolver) {
        this->initializeOplogs(numberOplogs, oplogFileNameResolver);
    }

    void addAll(const std::vector<OperationBody>& operations, const OperationLogOptions options = {}) override {
        for (const OperationBody& operation: operations) {
            this->add(operation, options);
        }
    }

    void add(const OperationBody& operation, const OperationLogOptions options = {}) override {
        int oplogId = this->oplogResolver(operation);
        singleOperationLog_t oplog = this->operationLogs[oplogId];

        oplog->add(operation, options);
    }

    bool hasOplogFile(const OperationLogOptions options = {}) override {
        return this->operationLogs.size() < options.operationLogId &&
            this->operationLogs.at(options.operationLogId)->hasOplogFile(options);
    }

    std::vector<OperationBody> clear(const OperationLogOptions options = {}) override {
        if(options.operationLogId >= this->operationLogs.size()){
            return std::vector<OperationBody>{};
        }

        singleOperationLog_t operationLogToClear = this->operationLogs.at(options.operationLogId);
        std::vector<OperationBody> operationsCleared = operationLogToClear->clear(options);

        this->operationLogs.erase(this->operationLogs.begin() + options.operationLogId);

        return operationsCleared;
    }

    std::vector<OperationBody> getAfterTimestamp(uint64_t timestamp, OperationLogOptions options) override {
        singleOperationLog_t oplog = this->operationLogs[options.operationLogId];

        return oplog->getAfterTimestamp(timestamp, options);
    }

    std::vector<OperationBody> get(const OperationLogOptions options = {}) override {
        std::vector<OperationBody> totalFromDisk{};

        return this->operationLogs.at(options.operationLogId)->get(options);
    }

    uint32_t getNumberOplogFiles() override {
        return this->operationLogs.size();
    }

private:
    void initializeOplogs(int numberOplogs, std::function<std::string(int)> oplogFileNameResolver) {
        for(int i = 0; i < numberOplogs; i++) {
            std::string fileName = oplogFileNameResolver(i);
            singleOperationLog_t oplog = std::make_shared<SingleOperationLog>(this->configuration, fileName);

            this->operationLogs.push_back(oplog);
        }
    }
};
