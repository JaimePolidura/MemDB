#pragma once

#include "shared.h"
#include "replication/clustermanager/responses/AllNodesResponse.h"
#include "replication/Replication.h"
#include "logging/Logger.h"

class ReplicationCreator {
public:
    static auto setup(configuration_t configuration, logger_t logger) -> replication_t {
        replication_t replication = std::make_shared<Replication>(logger, configuration);
        replication->setup(true);

        return replication;
    }
};