#pragma once

#include "shared.h"
#include "replication/clustermanager/requests/AllNodesResponse.h"
#include "replication/Replication.h"
#include "logging/Logger.h"

class ReplicationCreator {
public:
    static auto setup(configuration_t configuration, logger_t logger) -> replication_t {
        clusterManagerService_t clusterManagerService = std::make_shared<ClusterManagerService>(configuration, logger);
        AllNodesResponse allNodes = clusterManagerService->getAllNodes();
        logger->info("Recieved cluster information");

        replication_t replication = std::make_shared<Replication>(logger, configuration, clusterManagerService);
        replication->setClusterInformation(allNodes);
        replication->setBooting();
        replication->initialize();

        return replication;
    }
};