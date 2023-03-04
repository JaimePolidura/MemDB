#pragma once

#include <cstdlib>
#include <memory>
#include <vector>
#include <ctime>

#include "utils/clock/LamportClock.h"
#include "config/Configuration.h"
#include "replication/clustermanager/ClusterManagerService.h"
#include "NodeState.h"
#include "config/keys/ConfigurationKeys.h"
#include "replication/othernodes/ClusterNodesConnections.h"
#include "utils/strings/StringUtils.h"
#include "replication/clusterdb/ClusterDb.h"
#include "ClusterDbNodeChangeHandler.h"

class Replication {
private:
    configuration_t configuration;
    clusterNodesConnections_t clusterNodesConnections;
    clusterManagerService_t clusterManager;
    clusterdb_t clusterDb;
    ClusterDbNodeChangeHandler clusterDbNodeChangeHandler;
    NodeState state;
    int nodeId;

public:
    Replication(configuration_t configuration): configuration(configuration) {}

    Replication(configuration_t configuration, clusterManagerService_t clusterManager, SetupNodeResponse setupNodeResponse) :
            configuration(configuration), state(NodeState::BOOTING), nodeId(setupNodeResponse.nodeId),
            clusterNodesConnections(std::make_shared<ClusterNodesConnections>(configuration, setupNodeResponse.nodes)),
            clusterDb(std::make_shared<ClusterDb>(configuration)), clusterManager(clusterManager), clusterDbNodeChangeHandler(this->clusterNodesConnections)
    {}

    auto watchForChangesInClusterDb() -> void {
        this->watchForChangesInNodesInCluster();
    }

    auto initializeNodeConnections() -> void {
        this->clusterNodesConnections->createConnections();
    }

    auto doesAddressBelongToReplicationNode(const std::string& address) -> bool {
        for(const auto& node : this->clusterNodesConnections->otherNodes)
            if(node.address.compare(address) == 0)
                return true;

        return false;
    }

    auto getUnsyncedOpLogs(uint64_t lastTimestampProcessedFromOpLog) -> std::vector<OperationBody> {
        OperationLogDeserializer operationLogDeserializer{};

        auto responseFromSyncData = this->clusterNodesConnections->sendRequestToRandomNode(this->createSyncDataRequest(lastTimestampProcessedFromOpLog));
        uint8_t * begin = responseFromSyncData.responseValue.data();
        auto bytes = std::vector<uint8_t>(begin, begin + responseFromSyncData.responseValue.size);

        if(responseFromSyncData.isSuccessful)
            return operationLogDeserializer.deserializeAll(bytes);
        else
            throw std::runtime_error("Unexpected error in syncing data");
    }

    auto broadcast(const Request& request, const bool includeNodeId = true) -> void {
        this->clusterNodesConnections->broadcast(request, includeNodeId);
    }

    auto sendRequestToRandomNode(const Request& request) -> Response {
        return this->clusterNodesConnections->sendRequestToRandomNode(request);
    }

    auto getNodeId() -> uint16_t {
        return this->nodeId;
    }

private:
    auto watchForChangesInNodesInCluster() -> void {
        this->clusterDb->watch("/nodes", [this](ClusterDbValueChanged nodeChangedEvent){
            auto node = Node::fromJson(nodeChangedEvent.value);

            this->clusterDbNodeChangeHandler.handleChange(node, nodeChangedEvent.changeType);
        });
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

using replication_t = std::shared_ptr<Replication>;