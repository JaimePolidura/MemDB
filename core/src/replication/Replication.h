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
    Replication(logger_t logger, configuration_t configuration) :
            configuration(configuration),clusterDb(std::make_shared<ClusterDb>(configuration, logger)),
            clusterNodesConnections(std::make_shared<ClusterNodesConnections>(configuration)),
            clusterManager(std::make_shared<ClusterManagerService>(configuration, logger)),
            clusterDbNodeChangeHandler(this->clusterNodesConnections), logger(logger)
    {}

    auto setup(bool firstTime = false) -> void {
        this->logger->info("Setting up node in the cluster");

        this->setClusterInformation(this->clusterManager->getAllNodes());
        this->logger->info("Cluster information is set");

        this->setBooting();

        this->initializeNodeConnections();
        this->logger->info("Created connections to the rest of the nodes in the cluster");

        this->watchForChangesInClusterDb();
        this->logger->info("Started watching changes in the clusterdb");

        if(!firstTime){ //Network partitions?
            this->getUnsyncedOplog(this->lastTimestampBroadcasted);
            this->setRunning();
        }

        this->logger->info("Replication node is now set up");
    }

    auto getUnsyncedOplog(uint64_t lastTimestamp) -> std::vector<OperationBody> {
        this->logger->info("Synchronizing oplog with the cluster");
        auto unsyncedOps = this->getUnsyncedOpLogs(lastTimestamp);
        this->reloadUnsyncedOpsCallback(unsyncedOps);

        return unsyncedOps;
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
//        return NodeState::BOOTING;
    }

    auto getNodeId() -> std::string {
        return this->selfNode.nodeId;
//        return "ZD";
    }

private:
    auto setClusterInformation(AllNodesResponse allNodesResponse) -> void {
        std::string selfNodeId = this->configuration->get(ConfigurationKeys::NODE_ID);
        std::vector<Node> allNodes = allNodesResponse.nodes;

        this->selfNode = *std::find_if(allNodes.begin(), allNodes.end(), [&selfNodeId, this](Node node) -> bool{
            return node.nodeId == selfNodeId;
        });

        std::vector<Node> otherNodes;
        std::copy_if(allNodes.begin(), allNodes.end(), std::back_inserter(otherNodes), [selfNodeId](Node node) -> bool{
            return node.nodeId != selfNodeId;
        });

        this->clusterNodesConnections->setOtherNodes(otherNodes);
    }

    auto watchForChangesInNodesInCluster() -> void {
        this->clusterDb->watch("/nodes", [this](ClusterDbValueChanged nodeChangedEvent){
            auto node = Node::fromJson(nodeChangedEvent.value);
            auto selfNodeChanged = node.nodeId == this->selfNode.nodeId;

            if(!selfNodeChanged){
                this->clusterDbNodeChangeHandler.handleChange(node, nodeChangedEvent.changeType);
            }
            if(selfNodeChanged && (node.state == NodeState::SHUTDOWN || node.state == NodeState::BOOTING)) { //Reload
                this->setup(false);
                this->reloadUnsyncedOpsCallback(this->getUnsyncedOpLogs(this->lastTimestampBroadcasted));
            }
        });
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