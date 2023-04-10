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

#include "logging/Logger.h"

class Replication {
private:
    configuration_t configuration;
    clusterNodesConnections_t clusterNodesConnections;
    ClusterDbNodeChangeHandler clusterDbNodeChangeHandler;
    std::atomic_uint64_t lastTimestampBroadcasted;
    clusterManagerService_t clusterManager;
    clusterdb_t clusterDb;
    Node selfNode;
    logger_t logger;

    std::function<void(std::vector<OperationBody>)> reloadUnsyncedOpsCallback;

public:
    Replication(logger_t logger, configuration_t configuration): configuration(configuration), logger(logger) {}

    Replication(logger_t logger, configuration_t configuration, clusterManagerService_t clusterManager) :
            configuration(configuration), clusterDb(std::make_shared<ClusterDb>(configuration)),
            clusterNodesConnections(std::make_shared<ClusterNodesConnections>(configuration)),
            clusterManager(clusterManager), clusterDbNodeChangeHandler(this->clusterNodesConnections), logger(logger)
    {}

    auto setClusterInformation(AllNodesResponse allNodesResponse) -> void {
        std::string selfNodeId = this->configuration->get(ConfigurationKeys::NODE_ID);
        std::vector<Node> allNodes = allNodesResponse.nodes;

        this->selfNode = *std::find_if(allNodes.begin(), allNodes.end(), [&selfNodeId](Node node) -> bool{
            return node.nodeId == selfNodeId;
        });

        std::vector<Node> otherNodes;
        std::copy_if(allNodes.begin(), allNodes.end(), std::back_inserter(otherNodes), [selfNodeId](Node node) -> bool{
            return node.nodeId == selfNodeId;
        });
        this->clusterNodesConnections->setOtherNodes(otherNodes);
    }

    auto setReloadUnsyncedOpsCallback(std::function<void(std::vector<OperationBody>)> callback) -> void {
        this->reloadUnsyncedOpsCallback = callback;
    }

    auto setBooting() -> void {
        this->selfNode.state = NodeState::BOOTING;
        this->clusterDb->set(this->selfNode.nodeId, this->selfNode);
        this->logger->info("Changed replication node state to BOOTING");
    }

    auto setRunning() -> void {
        this->selfNode.state = NodeState::RUNNING;
        this->clusterDb->set(this->selfNode.nodeId, this->selfNode);
        this->logger->info("Changed replication node state to RUNNING");
    }

    auto initialize() -> void {
        this->initializeNodeConnections();
        this->logger->info("Created connections to cluster nodes");

        this->watchForChangesInClusterDb();
        this->logger->info("Started watching changes in the clusterdb");
    }

    auto doesAddressBelongToReplicationNode(const std::string& address) -> bool {
        for(const auto& node : this->clusterNodesConnections->otherNodes)
            if(node.address.compare(address) == 0)
                return true;

        return false;
    }

    auto getUnsyncedOpLogs(uint64_t lastTimestampProcessedFromOpLog) -> std::vector<OperationBody> {
        if(this->clusterNodesConnections->otherNodes.empty())
            return std::vector<OperationBody>{};

        OperationLogDeserializer operationLogDeserializer{};
        Response responseFromSyncData = this->clusterNodesConnections->sendRequestToRandomNode(createSyncDataRequest(lastTimestampProcessedFromOpLog));

        if(responseFromSyncData.responseValue.hasData())
            return std::vector<OperationBody>{};

        uint8_t * begin = responseFromSyncData.responseValue.data();
        auto bytes = std::vector<uint8_t>(begin, begin + responseFromSyncData.responseValue.size);

        return operationLogDeserializer.deserializeAll(bytes);
    }

    virtual auto broadcast(const Request& request, const bool includeNodeId = true) -> void {
        this->lastTimestampBroadcasted = request.operation.timestamp;
        this->clusterNodesConnections->broadcast(request, includeNodeId);
    }

    virtual auto getNodeState() -> NodeState {
        return this->selfNode.state;
    }

    auto getNodeId() -> std::string {
        return this->selfNode.nodeId;
    }

private:
    auto watchForChangesInNodesInCluster() -> void {
        this->clusterDb->watch("/nodes", [this](ClusterDbValueChanged nodeChangedEvent){
            auto node = Node::fromJson(nodeChangedEvent.value);
            auto selfNodeChanged = node.nodeId == this->selfNode.nodeId;

            if(!selfNodeChanged){
                this->clusterDbNodeChangeHandler.handleChange(node, nodeChangedEvent.changeType);
            }
            if(selfNodeChanged && (node.state == NodeState::SHUTDOWN || node.state == NodeState::BOOTING)) {
                this->reload();
            }
        });
    }

    auto reload() -> void {
        this->setBooting();
        this->setClusterInformation(this->clusterManager->getAllNodes());
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