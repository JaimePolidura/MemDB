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
    configuration_t configuration;
    ClusterNodesConnections clusterNodesConnections;
    ClusterManagerService clusterManager;
    NodeState state;
    int nodeId;

public:
    Replication(configuration_t configuration, int nodeId, const std::vector<Node>& otherNodes) :
            clusterManager(configuration), configuration(configuration), state(NodeState::BOOTING), nodeId(nodeId),
            clusterNodesConnections(configuration, otherNodes)
    {}

    bool doesBelongToReplicationNode(const std::string& address) {
        for(const auto& node : this->clusterNodesConnections.otherNodes)
            if(node.address.compare(address) == 0)
                return true;

        return false;
    }

    void broadcast(const Request& request, const bool includeNodeId = true) {
        this->clusterNodesConnections.broadcast(request, includeNodeId);
    }

    Response sendRequestToRandomNode(const Request& request) {
        return this->clusterNodesConnections.sendRequestToRandomNode(request);
    }

    uint16_t getNodeId() {
        return this->nodeId;
    }

private:
    Response sendSyncDataRequest(uint64_t timestamp) {
        return this->clusterNodesConnections.sendRequestToRandomNode(this->createSyncDataRequest(timestamp));
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