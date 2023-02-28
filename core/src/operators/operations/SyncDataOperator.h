#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "utils/Utils.h"
#include "persistence/OperationLogDiskLoader.h"
#include "operators/ControlOperator.h"

class SyncDataOperator : public Operator, public ControlOperator {
private:
    OperationLogDiskLoader operationLogDiskLoader;
    OperationLogSerializer operationLogSerializer;

public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x05;

    //TODO All the logs from disk might not ocuppy 2^32 bits
    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, operationLogBuffer_t operationLogBuffer) override {
        operationLogBuffer->lockWritesToDisk();

        uint64_t lastTimestampInClient = Utils::parse<uint64_t>(operation.args->at(0).data());
        uint64_t oldestTimestampInBuffer = operationLogBuffer->getFirstTimestampWritten();

        std::vector<uint8_t> operationsToSendToTheClient;
        
        if(oldestTimestampInBuffer != 0 && oldestTimestampInBuffer < lastTimestampInClient)
            this->takeOperationsFromBufferAndSerialize(lastTimestampInClient, operationsToSendToTheClient, operationLogBuffer);
        else
            this->takeOperationsFromDiskAndBufferAndSerialize(lastTimestampInClient, operationsToSendToTheClient, operationLogBuffer);

        return Response::success(1);
    }

    AuthenticationType authorizedToExecute() override {
        return AuthenticationType::CLUSTER;
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
        std::vector<OperationBody> operationLogsInDisk = operationLogDiskLoader.getAll();
        std::vector<OperationBody> operationLogsInBuffer =  operationLogBuffer->get();
        operationLogBuffer->unlockWritesToDisk();

        for(auto it = operationLogsInDisk.begin(); it < operationLogsInDisk.end() && it->timestamp > lastTimestampInClient; it++) //Very long log may collapse RAM TODO fix
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
        for(auto it = operationLogsInBuffer.begin(); it < operationLogsInBuffer.end(); it++)
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
    }

    void takeOperationsFromBufferAndSerialize(uint64_t lastTimestampInClient, std::vector<uint8_t>& operationsToSendToTheClient,
                                              operationLogBuffer_t operationLogBuffer) {
        operationLogBuffer->unlockWritesToDisk();
        std::vector<OperationBody> operationLogsInBuffer =  operationLogBuffer->get();

        for(auto it = operationLogsInBuffer.begin(); it < operationLogsInBuffer.end() && it->timestamp > lastTimestampInClient; it++)
            this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
    }
};