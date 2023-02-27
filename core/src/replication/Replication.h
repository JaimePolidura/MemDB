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

class Replication {
private:
    ClusterNodesConnections clusterNodesConnections;
    configuration_t configuration;
    ClusterManagerService clusterManager;
    NodeState state;
    int nodeId;

public:
    Replication() = default;

    Replication(configuration_t configuration, int nodeId, const std::vector<Node>& otherNodes) :
            clusterManager(configuration), configuration(configuration), state(NodeState::BOOTING), nodeId(nodeId)
    {
        this->clusterNodesConnections.createSocketsToNodes(otherNodes);
    }

    void setup(uint64_t lastTimestampProcessed) {
        auto responseSetup = this->clusterManager.setupNode();
        this->nodeId = responseSetup.nodeId;

        if(responseSetup.nodes.empty())
            return;

        this->clusterNodesConnections.createSocketsToNodes(responseSetup.nodes);

        auto syncDataResponse = this->sendSyncDataRequest(lastTimestampProcessed);
    }

    Response sendRequest(const Request& request) {
        return this->clusterNodesConnections.sendRequest(request);
    }

    uint16_t getNodeId() {
        return this->nodeId;
    }

private:
    Response sendSyncDataRequest(uint64_t timestamp) {
        return this->clusterNodesConnections.sendRequest(this->createSyncDataRequest(timestamp));
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