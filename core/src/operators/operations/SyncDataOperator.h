#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "utils/Utils.h"
#include "persistence/OperationLogDiskLoader.h"

#include <string>

class SyncDataOperator : public Operator {
public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x05;

    OperationLogDiskLoader operationLogDiskLoader;
    OperationLogSerializer operationLogSerializer;

    //TODO All the logs from disk might not ocuppy 2^32 bits
    Response operateControl(const OperationBody& operation, const OperationOptions& operationOptions, std::shared_ptr<OperationLogBuffer> operationLogBuffer) override {
        operationLogBuffer->lockWritesToDisk();
        std::vector<OperationBody> operationLogsInBuffer = operationLogBuffer->get();

        uint64_t lastTimestampInClient = Utils::parse<uint64_t>(operation.args->at(0).data());
        uint64_t oldestTimestampInBuffer = operationLogBuffer->getFirstTimestampWritten();

        std::vector<uint8_t> operationsToSendToTheClient;
        
        if(oldestTimestampInBuffer != 0 && oldestTimestampInBuffer < lastTimestampInClient) {
            //Take operations from item lastTimestampInClient + 1 to last item in buffer
            operationLogBuffer->unlockWritesToDisk();

            for(auto it = operationLogsInBuffer.begin(); it < operationLogsInBuffer.end() && it->timestamp > lastTimestampInClient; it++)
                this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);

        } else {
            //Read from disk operations logs and add all the operations in the buffer
            std::vector<OperationBody> operationLogsInDisk = operationLogDiskLoader.getAll();
            operationLogBuffer->unlockWritesToDisk();

            for(auto it = operationLogsInDisk.begin(); it < operationLogsInDisk.end() && it->timestamp > lastTimestampInClient; it++) //Very long log may collapse RAM TODO fix
                this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
            for(auto it = operationLogsInBuffer.begin(); it < operationLogsInBuffer.end(); it++)
                this->operationLogSerializer.serialize(operationsToSendToTheClient, * it);
        }

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
};