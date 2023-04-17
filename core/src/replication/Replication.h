#pragma once

#include "shared.h"

#include "replication/clustermanager/ClusterManagerService.h"
#include "replication/NodeState.h"
#include "replication/othernodes/ClusterNodes.h"
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
    clusterNodes_t clusterNodes;
    ClusterDbNodeChangeHandler clusterDbNodeChangeHandler;
    std::atomic_uint64_t lastTimestampBroadcasted;
    clusterManagerService_t clusterManager;
    clusterdb_t clusterDb;
    node_t selfNode;
    logger_t logger;

    std::function<void(std::vector<OperationBody>)> reloadUnsyncedOplogCallback;

public:
    Replication(logger_t logger, configuration_t configuration) :
            configuration(configuration), clusterDb(std::make_shared<ClusterDb>(configuration, logger)),
            clusterNodes(std::make_shared<ClusterNodes>(configuration, logger)),
            clusterManager(std::make_shared<ClusterManagerService>(configuration, logger)),
            clusterDbNodeChangeHandler(this->clusterNodes, logger), logger(logger)
    {}

    auto setup(bool firstTime = false) -> void {
        this->logger->info("Setting up node in the cluster");

        this->setClusterInformation(this->clusterManager->getAllNodes());
        this->watchForChangesInClusterDb();
        this->setBooting();

        if(!firstTime){ //Network partitions?
            auto unsyncedOplog = this->getUnsyncedOplog(this->lastTimestampBroadcasted);
            this->reloadUnsyncedOplogCallback(unsyncedOplog);
            this->logger->info("Synchronized {0} oplog entries with the cluster", unsyncedOplog.size());
        }

        this->logger->info("Replication node is now set up");
    }

    auto getClusterNodes() -> clusterNodes_t {
        return this->clusterNodes;
    }

    auto setReloadUnsyncedOplogCallback(std::function<void(std::vector<OperationBody>)> callback) -> void {
        this->reloadUnsyncedOplogCallback = callback;
    }

    auto setBooting() -> void {
        this->selfNode->state = NodeState::BOOTING;
        this->clusterDb->setNode(this->selfNode->nodeId, this->selfNode);
        this->logger->info("Changed replication node state to BOOTING");
    }

    auto setRunning() -> void {
        this->selfNode->state = NodeState::RUNNING;
        this->clusterDb->setNode(this->selfNode->nodeId, this->selfNode);
        this->logger->info("Changed replication node state to RUNNING");
    }

    auto doesAddressBelongToReplicationNode(const std::string& address) -> bool {
        for(const auto& node : this->clusterNodes->otherNodes)
            if(node->address.compare(address) == 0)
                return true;

        return false;
    }

    auto getUnsyncedOplog(uint64_t lastTimestampProcessedFromOpLog) -> std::vector<OperationBody> {
        if(this->clusterNodes->otherNodes.empty())
            return std::vector<OperationBody>{};

        OperationLogDeserializer operationLogDeserializer{};
        std::optional<Response> responseFromSyncDataOpt = this->clusterNodes->sendRequestToRandomNode(createSyncOplogRequest(lastTimestampProcessedFromOpLog));

        if(!responseFromSyncDataOpt.has_value() || !responseFromSyncDataOpt.value().responseValue.hasData()) {
            return std::vector<OperationBody>{};
        }

        Response responseFromSyncData = responseFromSyncDataOpt.value();
        uint8_t * begin = responseFromSyncData.responseValue.data();
        auto bytes = std::vector<uint8_t>(begin, begin + responseFromSyncData.responseValue.size);

        return operationLogDeserializer.deserializeAll(bytes);
    }

    virtual auto broadcast(const Request& request) -> void {
        this->lastTimestampBroadcasted = request.operation.timestamp;
        this->clusterNodes->broadcast(request);
    }

    virtual auto getNodeState() -> NodeState {
        return this->selfNode->state;
    }
    
    auto getNodeId() -> memdbNodeId_t {
        return this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);
    }

private:
    auto setClusterInformation(AllNodesResponse allNodesResponse) -> void {
        memdbNodeId_t selfNodeId = this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);
        std::vector<node_t> allNodes = allNodesResponse.nodes;

        this->selfNode = *std::find_if(allNodes.begin(), allNodes.end(), [selfNodeId](node_t node) -> bool{
            return node->nodeId == selfNodeId;
        });

        std::vector<node_t> otherNodes;
        std::copy_if(allNodes.begin(), allNodes.end(), std::back_inserter(otherNodes), [selfNodeId](node_t node) -> bool{
            return node->nodeId != selfNodeId;
        });

        this->clusterNodes->setOtherNodes(otherNodes);

        this->logger->info("Cluster information is set. Total all nodes {0}", allNodesResponse.nodes.size());
    }

    auto createSyncOplogRequest(uint64_t timestamp) -> Request {
        auto authenticationBody = AuthenticationBody{this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY), false, false};
        auto argsVector = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();

        argsVector->push_back(SimpleString<memDbDataLength_t>::fromString(StringUtils::toString(timestamp)));

        OperationBody operationBody{};
        operationBody.args = argsVector;
        operationBody.operatorNumber = 0x05; //SyncOplogOperator operator number

        Request request{};
        request.operation = operationBody;
        request.authentication = authenticationBody;

        return request;
    }

    auto watchForChangesInClusterDb() -> void {
        this->clusterDb->watchNodeChanges([this](ClusterDbValueChanged nodeChangedEvent) {
            auto node = Node::fromJson(nodeChangedEvent.value);
            auto selfNodeChanged = node->nodeId == this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);

            this->logger->debugInfo("Detected change of type {0} on node {1}", ClusterDbChangeTypes::toString(nodeChangedEvent.changeType), node->nodeId);

            if (!selfNodeChanged) {
                this->clusterDbNodeChangeHandler.handleChange(node, nodeChangedEvent.changeType);
            }
            if (selfNodeChanged && node->state == NodeState::SHUTDOWN) { //Reload
                this->logger->info("Reload detected node in the cluster");
                this->setup(false);
            }
        });
    }
};

using  replication_t = std::shared_ptr<Replication>;