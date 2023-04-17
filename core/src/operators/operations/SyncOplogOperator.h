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
        std::cout << "1" << std::endl;
        operationLogBuffer->lockWritesToDisk();
        std::cout << "2" << std::endl;

        uint64_t lastTimestampInClient = Utils::parse<uint64_t>(operation.args->at(0).data());
        std::cout << "3" << std::endl;

        uint64_t oldestTimestampInBuffer = operationLogBuffer->getFirstTimestampWritten();
        std::cout << "4" << std::endl;

        std::vector<uint8_t> operationsToSendToTheClient;
        std::cout << "5" << std::endl;

        if(oldestTimestampInBuffer != 0 && oldestTimestampInBuffer < lastTimestampInClient)
            this->takeOperationsFromBufferAndSerialize(lastTimestampInClient, operationsToSendToTheClient, operationLogBuffer);
        else
            this->takeOperationsFromDiskAndBufferAndSerialize(lastTimestampInClient, operationsToSendToTheClient, operationLogBuffer);

//        return Response::success(SimpleString<memDbDataLength_t>::fromArray(operationsToSendToTheClient), 1);
        return Response::success(1);
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
    void takeOperationsFromDiskAndBufferAndSerialize(uint64_t lastTimestampInClient, std::vector<uint8_t>& operationsToSendToTheClient,
                                                     operationLogBuffer_t operationLogBuffer) {
        std::cout << "6" << std::endl;

        std::vector<OperationBody> operationLogsInDisk = operationLogDiskLoader.getAll();
        std::cout << "7" << std::endl;
        std::vector<OperationBody> operationLogsInBuffer =  operationLogBuffer->get();
        std::cout << "8" << std::endl;
        operationLogBuffer->unlockWritesToDisk();
        std::cout << "9" << std::endl;

        auto compactedFromDisk = this->operationLogCompacter.compact(operationLogsInDisk);
        std::cout << "10" << std::endl;
        auto compactedFromBuffer = this->operationLogCompacter.compact(operationLogsInBuffer);
        std::cout << "11" << std::endl;

        for(auto it = compactedFromDisk.begin(); it < compactedFromDisk.end() && it->timestamp > lastTimestampInClient; it++) //Very long log may collapse RAM TODO fix
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
        for(auto it = compactedFromBuffer.begin(); it < compactedFromBuffer.end(); it++)
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);

        std::cout << "12" << std::endl;
    }

    void takeOperationsFromBufferAndSerialize(uint64_t lastTimestampInClient,
                                              std::vector<uint8_t>& operationsToSendToTheClient,
                                              operationLogBuffer_t operationLogBuffer) {
        operationLogBuffer->unlockWritesToDisk();
        std::vector<OperationBody> operationLogsInBuffer = operationLogBuffer->get();

        for(auto it = operationLogsInBuffer.begin(); it < operationLogsInBuffer.end() && it->timestamp > lastTimestampInClient; it++)
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
    }
};