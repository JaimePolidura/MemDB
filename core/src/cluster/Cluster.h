#pragma once

#include "shared.h"

#include "cluster/clustermanager/ClusterManagerService.h"
#include "cluster/partitions/Partitions.h"
#include "cluster/NodeState.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "cluster/clusterdb/ClusterDb.h"
#include "cluster/clusterdb/changehandler/ClusterDbNodeChangeHandler.h"

#include "utils/clock/LamportClock.h"
#include "utils/strings/StringUtils.h"

#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"

#include "logging/Logger.h"

class Cluster {
private:
    configuration_t configuration;
    clusterNodes_t clusterNodes;
    clusterDbNodeChangeHandler_t clusterDbNodeChangeHandler;
    clusterManagerService_t clusterManager;
    partitions_t partitions;
    clusterdb_t clusterDb;
    node_t selfNode;
    logger_t logger;

    friend class ClusterNodeSetup;
    friend class SimpleClusterNodeSetup;
    friend class PartitionsClusterNodeSetup;

public:
    Cluster() = default;

    Cluster(logger_t logger, configuration_t configuration) :
            configuration(configuration), clusterDb(std::make_shared<ClusterDb>(configuration, logger)),
            clusterNodes(std::make_shared<ClusterNodes>(configuration, logger)),
            clusterManager(std::make_shared<ClusterManagerService>(configuration, logger)), logger(logger)
    {}

    auto setBooting() -> void {
        this->selfNode->state = NodeState::BOOTING;
        this->clusterDb->setNode(this->selfNode->nodeId, this->selfNode);
        this->logger->info("Changed cluster node state to BOOTING");
    }

    auto setRunning() -> void {
        this->selfNode->state = NodeState::RUNNING;
        this->clusterDb->setNode(this->selfNode->nodeId, this->selfNode);
        this->logger->info("Changed cluster node state to RUNNING");
    }

    auto getUnsyncedOplog(uint64_t lastTimestampProcessedFromOpLog) -> std::vector<OperationBody> {
        if(this->clusterNodes->otherNodes.empty())
            return std::vector<OperationBody>{};

        OperationLogDeserializer operationLogDeserializer{};
        std::optional<Response> responseFromSyncDataOpt = this->clusterNodes->sendRequestToRandomNode(createSyncOplogRequest(lastTimestampProcessedFromOpLog));

        if(!responseFromSyncDataOpt.has_value() || !responseFromSyncDataOpt.value().responseValue.hasData())
            return std::vector<OperationBody>{};

        Response responseFromSyncData = responseFromSyncDataOpt.value();
        uint8_t * begin = responseFromSyncData.responseValue.data();
        auto bytes = std::vector<uint8_t>(begin, begin + responseFromSyncData.responseValue.size);

        return operationLogDeserializer.deserializeAll(bytes);
    }

    auto getPartitionObject() -> partitions_t {
        return this->partitions;
    }

    virtual auto broadcast(const OperationBody& operation) -> void {
        this->clusterNodes->broadcast(operation);
    }

    virtual auto getNodeState() -> NodeState {
        return this->selfNode->state;
    }

private:
    auto watchForChangesInNodesClusterDb() -> void {
        this->clusterDb->watchNodeChanges([this](ClusterDbValueChanged nodeChangedEvent) {
            auto node = Node::fromJson(nodeChangedEvent.value);
            auto selfNodeChanged = node->nodeId == this->configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID);

            if (!selfNodeChanged) {
                this->clusterDbNodeChangeHandler->handleChange(node, nodeChangedEvent.changeType);
            }
            if (selfNodeChanged && node->state == NodeState::SHUTDOWN) { //Reload
                this->setRunning();
            }
        });
    }

    auto createSyncOplogRequest(uint64_t timestamp) -> Request {
        auto authenticationBody = AuthenticationBody{this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY), false, false};
        auto argsVector = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();

        uint32_t part1 = timestamp >> 32;
        uint32_t part2 = (uint32_t) timestamp & 0xFFFFFFFF;

        argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(part1));
        argsVector->push_back(SimpleString<memDbDataLength_t>::fromNumber(part2));

        OperationBody operationBody{};
        operationBody.args = argsVector;
        operationBody.operatorNumber = 0x05; //SyncOplogOperator operator number

        Request request{};
        request.operation = operationBody;
        request.authentication = authenticationBody;

        return request;
    }
};

using cluster_t = std::shared_ptr<Cluster>;