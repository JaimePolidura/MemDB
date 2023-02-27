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

class Replication {
private:
    ClusterNodesConnections clusterNodesConnections;
    configuration_t configuration;
    ClusterManagerService clusterManager;

    lamportClock_t clock;
    NodeState state;
    int nodeId;

public:
    Replication(lamportClock_t clock, configuration_t configuration) :
        nodeId(1), clock(clock), configuration(configuration), clusterManager(configuration), state(NodeState::BOOTING){
    }

    void setup(uint64_t lastTimestampProcessed) {
        auto responseSetup = this->clusterManager.setupNode();
        this->nodeId = responseSetup.nodeId;

        if(responseSetup.nodes.empty())
            return;

        this->clusterNodesConnections.createSocketsToNodes(responseSetup.nodes);
    }

    uint16_t getNodeId() {
        return this->nodeId;
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }
};

using replication_t = std::shared_ptr<Replication>;