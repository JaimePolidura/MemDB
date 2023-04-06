#pragma once

#include "shared.h"

#include "replication/clustermanager/ClusterManagerService.h"
#include "replication/NodeState.h"
#include "replication/othernodes/ClusterNodesConnections.h"
#include "replication/clusterdb/ClusterDb.h"
#include "replication/ClusterDbNodeChangeHandler.h"

#include "utils/clock/LamportClock.h"
#include "utils/strings/StringUtils.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

class Replication {
private:
    configuration_t configuration;
    clusterNodesConnections_t clusterNodesConnections;
    ClusterDbNodeChangeHandler clusterDbNodeChangeHandler;
    std::atomic_uint64_t lastTimestampBroadcasted;
    clusterManagerService_t clusterManager;
    clusterdb_t clusterDb;
    Node selfNode;

    std::function<void(std::vector<OperationBody>)> reloadUnsyncedOpsCallback;

public:
    Replication(configuration_t configuration): configuration(configuration) {}

    Replication(configuration_t configuration, clusterManagerService_t clusterManager, InfoNodeResponse infoNodeResponse) :
            configuration(configuration), selfNode(infoNodeResponse.self), clusterDb(std::make_shared<ClusterDb>(configuration)),
            clusterNodesConnections(std::make_shared<ClusterNodesConnections>(configuration, infoNodeResponse.otherNodes)),
            clusterManager(clusterManager), clusterDbNodeChangeHandler(this->clusterNodesConnections)
    {}

    auto setReloadUnsyncedOpsCallback(std::function<void(std::vector<OperationBody>)> callback) -> void {
        this->reloadUnsyncedOpsCallback = callback;
    }

    auto setBooting() -> void {
        this->selfNode.state = NodeState::BOOTING;
        this->clusterDb->set(this->selfNode.nodeId, this->selfNode);
    }

    auto setRunning() -> void {
        this->selfNode.state = NodeState::RUNNING;
        this->clusterDb->set(this->selfNode.nodeId, this->selfNode);
    }

    auto initialize() -> void {
        this->watchForChangesInClusterDb();
        this->initializeNodeConnections();
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

    virtual auto broadcast(const Request& request, const bool includeNodeId = true) -> void {
        this->lastTimestampBroadcasted = request.operation.timestamp;
        this->clusterNodesConnections->broadcast(request, includeNodeId);
    }

    virtual auto getNodeState() -> NodeState {
        return this->selfNode.state;
    }

    auto getNodeId() -> uint16_t {
        return this->selfNode.nodeId;
    }

private:
    auto watchForChangesInNodesInCluster() -> void {
        this->clusterDb->watch("/nodes", [this](ClusterDbValueChanged nodeChangedEvent){
            auto node = Node::fromJson(nodeChangedEvent.value);
            auto sameNodeChanged = node.nodeId == this->selfNode.nodeId;

            if(!sameNodeChanged){
                this->clusterDbNodeChangeHandler.handleChange(node, nodeChangedEvent.changeType);
            }
            if(sameNodeChanged && (node.state == NodeState::SHUTDOWN || node.state == NodeState::BOOTING)) {
                this->reload();
            }
        });
    }

    auto reload() -> void {
        auto infoNodeRepsonse = this->clusterManager->getInfoNode();
        this->selfNode = infoNodeRepsonse.self;
        this->clusterNodesConnections->otherNodes = infoNodeRepsonse.otherNodes;
        this->initialize();

        auto unsyncedOps = this->getUnsyncedOpLogs(this->lastTimestampBroadcasted);
        this->reloadUnsyncedOpsCallback(unsyncedOps);
        this->setRunning();
    }

    auto createSyncDataRequest(uint64_t timestamp) -> Request {
        auto authenticationBody = AuthenticationBody{this->configuration->get(ConfigurationKeys::AUTH_CLUSTER_KEY), false, false};
        auto argsVector = std::make_shared<std::vector<SimpleString<defaultMemDbLength_t>>>();

        argsVector->push_back(SimpleString<defaultMemDbLength_t>::fromString(StringUtils::toString(timestamp)));

        OperationBody operationBody{};
        operationBody.args = argsVector;
        operationBody.operatorNumber = 0x05; //SyncDataOperator operator number

        Request request{};
        request.operation = operationBody;
        request.authentication = authenticationBody;

        return request;
    }

    auto watchForChangesInClusterDb() -> void {
        this->watchForChangesInNodesInCluster();
    }

    auto initializeNodeConnections() -> void {
        this->clusterNodesConnections->deleteAllConnections();
        this->clusterNodesConnections->createConnections();
    }
};

using replication_t = std::shared_ptr<Replication>;