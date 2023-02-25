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

class ReplicationNode {
private:
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<LamportClock> clock;
    std::vector<Node> otherNodes;
    ClusterManagerService clusterManager;
    NodeState state;
    int nodeId;

public:
    ReplicationNode(std::shared_ptr<LamportClock> clock, std::shared_ptr<Configuration> configuration) :
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