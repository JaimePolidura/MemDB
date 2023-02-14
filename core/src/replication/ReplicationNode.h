#pragma once

#include <memory>

#include "utils/clock/LamportClock.h"
#include "config/Configuration.h"
#include "NodeState.h"

class ReplicationNode {
private:
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<LamportClock> clock;
    NodeState state;
    int32_t nodeId;

public:
    ReplicationNode(std::shared_ptr<LamportClock> clock, std::shared_ptr<Configuration> configuration) :
        nodeId(-1), clock(clock), configuration(configuration) {}

    uint16_t getNodeId() {
        if(this->nodeId != -1)
            return this->nodeId;
        if(!this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION))
            this->nodeId = 1;
        if(this->configuration->getBoolean(ConfigurationKeys::USE_REPLICATION))
            this->nodeId = this->getFromClusterDb();

        return this->nodeId;
    }

    uint64_t tick(uint64_t other) {
        return this->clock->tick(other);
    }

private:
    int32_t getFromClusterDb() {
        return 0;
    }
};