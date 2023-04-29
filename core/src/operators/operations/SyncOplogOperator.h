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

    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, operationLog_t operationLog) override {
        uint64_t lastTimestampUnsync = parseUnsyncTimestampFromRequest(operation);

        std::vector<OperationBody> unsyncedOplog = operationLog->getAfterTimestamp(lastTimestampUnsync);
        std::vector<uint8_t> serializedUnsyncedOpLog = this->serializeOperations(unsyncedOplog);

        return Response::success(SimpleString<memDbDataLength_t>::fromVector(serializedUnsyncedOpLog));
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
    std::vector<uint8_t> serializeOperations(const std::vector<OperationBody>& operations) {
        std::vector<uint8_t> serialized{};
        for(auto it = operations.begin(); it < operations.end(); it++)
            this->operationLogSerializer.serialize(serialized, * it);

        return serialized;
    }

    //Timestamp is 64 bits Axtual memdb data size is 32 bits. Doest fit, we pass two args that consist of the two parts
    uint64_t parseUnsyncTimestampFromRequest(const OperationBody &operation) const {
        uint32_t part1 = Utils::parse<uint32_t>(operation.args->at(0).data());
        uint32_t part2 = Utils::parse<uint32_t>(operation.args->at(1).data());

        return part1 << 32 | part2;
    }
};