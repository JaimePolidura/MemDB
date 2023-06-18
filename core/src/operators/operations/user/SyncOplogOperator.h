#pragma once

#include "operators/Operator.h"
#include "messages/response/ErrorCode.h"
#include "utils/Utils.h"
#include "persistence/OperationLogSerializer.h"
#include "persistence/compaction/OperationLogCompacter.h"
#include "config/keys/ConfigurationKeys.h"

/**
 * Args:
 *  - uint32 part1 Timestamp to sync
 *  - uint32 part2 Timestamp to sync
 *  - uint32 nodeOplogIdToSync
 */
class SyncOplogOperator : public Operator {
private:
    OperationLogSerializer operationLogSerializer;
    OperationLogCompacter operationLogCompacter;

public:
    static constexpr const uint8_t OPERATOR_NUMBER = 0x05;

    Response operate(const OperationBody& operation, const OperationOptions options, OperatorDependencies dependencies) override {
        uint64_t lastTimestampUnsync = parseUnsyncTimestampFromRequest(operation);
        uint32_t nodeOplogIdToSync = this->calculateSelfOplogIdFromNodeOplogId(operation, dependencies) ;

        std::vector<OperationBody> unsyncedOplog = dependencies.operationLog->getAfterTimestamp(lastTimestampUnsync, OperationLogOptions{
            .operationLogId = nodeOplogIdToSync
        });

        unsyncedOplog = this->operationLogCompacter.compact(unsyncedOplog);

        std::vector<uint8_t> serializedUnsyncedOpLog = this->operationLogSerializer.serializeAll(unsyncedOplog);

        return Response::success(SimpleString<memDbDataLength_t>::fromVector(serializedUnsyncedOpLog));
    }

    std::vector<OperatorDependency> dependencies() override {
        return { OperatorDependency::OPERATION_LOG, OperatorDependency::CONFIGURATION, OperatorDependency::CLUSTER };
    }

    std::vector<AuthenticationType> authorizedToExecute() override {
        return { AuthenticationType::NODE };
    }

    constexpr OperatorType type() override {
        return OperatorType::NODE_MAINTENANCE;
    }

    constexpr uint8_t operatorNumber() override {
        return OPERATOR_NUMBER;
    }

    std::string name() override {
        return "SYNC_OPLOG";
    }

private:
    //Timestamp is 64 bits Actual memdb data size is 32 bits. Doest fit, we pass two args that consist of the two parts
    uint64_t parseUnsyncTimestampFromRequest(const OperationBody &operation) const {
        uint32_t part1 = Utils::parse<uint32_t>(operation.args->at(0).data());
        uint32_t part2 = Utils::parse<uint32_t>(operation.args->at(1).data());

        return part1 << 32 | part2;
    }

    uint32_t calculateSelfOplogIdFromNodeOplogId(const OperationBody &body, OperatorDependencies dependencies) {
        uint32_t nodeOplogId = body.getArg(2).to<uint32_t>();

        if(!dependencies.configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
            return nodeOplogId;
        }

        memdbNodeId_t otherNodeId = body.nodeId;
        int distance = dependencies.cluster->getPartitionObject()->getDistance(otherNodeId);

        return otherNodeId - distance;
    }
};