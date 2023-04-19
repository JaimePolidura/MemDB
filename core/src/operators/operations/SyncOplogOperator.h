#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "utils/Utils.h"
#include "persistence/OperationLogDiskLoader.h"
#include "operators/MaintenanceOperatorExecutor.h"
#include "persistence/compaction/OperationLogCompacter.h"

class SyncOplogOperator : public Operator, public MaintenanceOperatorExecutor {
private:
    OperationLogDiskLoader operationLogDiskLoader;
    OperationLogSerializer operationLogSerializer;
    OperationLogCompacter operationLogCompacter;

public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x05;

    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, operationLogBuffer_t operationLogBuffer) override {
        operationLogBuffer->lockWritesToDisk();

        uint64_t lastTimestampUnsync = parseUnsyncTimestampFromRequest(operation);
        uint64_t oldestTimestampInBuffer = operationLogBuffer->getOldestTimestampAdded();
        uint64_t lastestTimestampInBuffer = operationLogBuffer->getLatestTimestampAdded();
        bool bufferEmtpy = lastestTimestampInBuffer == 0 || oldestTimestampInBuffer == 0;

        if(!bufferEmtpy && lastestTimestampInBuffer <= lastTimestampUnsync){ //Already in sync
            return Response::success();
        }

        std::vector<uint8_t> operationsToSendToTheClient;
        if(!bufferEmtpy && oldestTimestampInBuffer <= lastTimestampUnsync)
            this->takeOperationsFromBufferAndSerialize(lastTimestampUnsync, operationsToSendToTheClient, operationLogBuffer);
        else
            this->takeOperationsFromDiskAndBufferAndSerialize(lastTimestampUnsync, operationsToSendToTheClient, operationLogBuffer);

        return Response::success(SimpleString<memDbDataLength_t>::fromVector(operationsToSendToTheClient));
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::NODE };
    }

    std::string name() override {
        return "SYNC_OPLOG";
    }

    constexpr OperatorType type() override {
        return OperatorType::CONTROL;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

private:
    void takeOperationsFromDiskAndBufferAndSerialize(uint64_t lastTimestampInClient,
                                                     std::vector<uint8_t>& operationsToSendToTheClient,
                                                     operationLogBuffer_t operationLogBuffer) {
        std::vector<OperationBody> operationLogsInDisk = operationLogDiskLoader.getAll();
        std::vector<OperationBody> operationLogsInBuffer =  operationLogBuffer->get();
        operationLogBuffer->unlockWritesToDisk();

        auto compactedFromDisk = this->operationLogCompacter.compact(operationLogsInDisk);
        auto compactedFromBuffer = this->operationLogCompacter.compact(operationLogsInBuffer);

        for(auto it = compactedFromDisk.begin(); it < compactedFromDisk.end() && it->timestamp > lastTimestampInClient; it++)  //Very long log may collapse RAM TODO fix
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
        for(auto it = compactedFromBuffer.begin(); it < compactedFromBuffer.end(); it++)
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
    }

    void takeOperationsFromBufferAndSerialize(uint64_t lastTimestampInClient,
                                              std::vector<uint8_t>& operationsToSendToTheClient,
                                              operationLogBuffer_t operationLogBuffer) {
        operationLogBuffer->unlockWritesToDisk();
        std::vector<OperationBody> operationLogsInBuffer = operationLogBuffer->get();

        for(auto it = operationLogsInBuffer.begin(); it < operationLogsInBuffer.end() && it->timestamp > lastTimestampInClient; it++)
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
    }

    //Timestamp is 64 bits Axtual memdb data size is 32 bits. Doest fit, we pass two args that consist of the two parts
    uint64_t parseUnsyncTimestampFromRequest(const OperationBody &operation) const {
        uint32_t part1 = Utils::parse<uint32_t>(operation.args->at(0).data());
        uint32_t part2 = Utils::parse<uint32_t>(operation.args->at(1).data());

        return ((uint32_t) part1) << 32 | part2;
    }

    uint64_t getLatestTimestampFromOplog() {
        std::vector<OperationBody> operationLogsInDisk = operationLogDiskLoader.getAll();
        return !operationLogsInDisk.empty() ? operationLogsInDisk.at(operationLogsInDisk.size() - 1).timestamp : 0;
    }
};