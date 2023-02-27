#pragma once

#include <cstdint>
#include "replication/Replication.h"

class ReplicationNodeStarter {
private:
    OperationLogDeserializer operationLogDeserializer;
    configuration_t configuration;
    ClusterManagerService clusterManagerService;

public:
    ReplicationNodeStarter(configuration_t configuration) : configuration(configuration),  clusterManagerService(configuration) {}

    auto setup(uint64_t lastTimestampProcessedFromOpLog) -> replication_t {
        auto setupResponse = this->clusterManagerService.setupNode();

        return std::make_shared<Replication>(configuration, setupResponse.nodeId, setupResponse.nodes);
    }

    auto getUnsyncedOpLogs(replication_t replication, uint64_t lastTimestampProcessedFromOpLog) -> std::vector<OperationBody> {
        auto responseFromSyncData = this->sendSyncDataRequest(replication, lastTimestampProcessedFromOpLog);
        uint8_t * begin = responseFromSyncData.responseValue.data();
        auto bytes = std::vector<uint8_t>(begin, begin + responseFromSyncData.responseValue.size);

        if(responseFromSyncData.isSuccessful)
            return this->operationLogDeserializer.deserializeAll(bytes);
        else
            throw std::runtime_error("Unexpected error in syncing data");
    }

private:
    Response sendSyncDataRequest(replication_t replication, uint64_t lastTimestampProcessedFromOpLog) {
        return replication->sendRequest(this->createSyncDataRequest(lastTimestampProcessedFromOpLog));
    }

    Request createSyncDataRequest(uint64_t timestamp) {
        auto authenticationBody = AuthenticationBody{this->configuration->get(ConfigurationKeys::AUTH_CLUSTER_KEY), false, false};
        auto argsVector = std::make_shared<std::vector<SimpleString<defaultMemDbSize_t>>>();

        argsVector->push_back(SimpleString<defaultMemDbSize_t>::fromString(StringUtils::toString(timestamp)));

        OperationBody operationBody{};
        operationBody.args = argsVector;
        operationBody.operatorNumber = 0x05; //SyncDataOperator operator number

        Request request{};
        request.operation = operationBody;
        request.authentication = authenticationBody;

        return request;
    }
};