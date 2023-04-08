#pragma once

#include "shared.h"
#include "replication/Replication.h"
#include "logging/Logger.h"

class ReplicationCreator {
public:
    static auto setup(configuration_t configuration, logger_t logger) -> replication_t {
        clusterManagerService_t clusterManagerService = std::make_shared<ClusterManagerService>(configuration, logger);
        InfoNodeResponse infoNodeResponse = clusterManagerService->getInfoNode();

        replication_t replication = std::make_shared<Replication>(logger, configuration, clusterManagerService, infoNodeResponse);
        replication->setBooting();
        replication->initialize();

        return replication;
    }
};