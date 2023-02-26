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

class Replication {
private:
    ClusterManagerService clusterManager;
    configuration_t configuration;
    std::vector<Node> otherNodes;
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
        this->otherNodes = responseSetup.nodes;

        if(this->otherNodes.empty())
            return;

        std::srand(std::time(nullptr));
        Node nodeToGetData = this->otherNodes[std::rand() % this->otherNodes.size()];
    }

    uint16_t getNodeId() {
        return this->nodeId;
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }
};

using replication_t = std::shared_ptr<Replication>;