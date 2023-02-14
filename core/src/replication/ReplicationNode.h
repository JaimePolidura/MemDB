#pragma once

#include <memory>

#include "utils/clock/LamportClock.h"
#include "config/Configuration.h"
#include "ClusterManagerService.h"
#include "NodeState.h"

struct Node {
    std::string address;
    NodeState state;
    int32_t nodeId;
};

class ReplicationNode {
private:
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<LamportClock> clock;
    ClusterManagerService clusterManager;
    NodeState state;
    int32_t nodeId;

public:
    ReplicationNode(std::shared_ptr<LamportClock> clock, std::shared_ptr<Configuration> configuration) :
        nodeId(-1), clock(clock), configuration(configuration), clusterManager(configuration), state(NodeState::BOOTING) {}

    uint16_t getNodeId() {
        if(this->nodeId != -1)
            return this->nodeId;
        if(!this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION))
            this->nodeId = 1;
        if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION))
            this->nodeId = this->clusterManager.getNodeId();

        return this->nodeId;
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }
};