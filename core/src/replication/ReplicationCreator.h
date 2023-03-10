#pragma once

#include "shared.h"
#include "replication/Replication.h"

class ReplicationCreator {
public:
    static auto setup(configuration_t configuration) -> replication_t {
        clusterManagerService_t clusterManagerService = std::make_shared<ClusterManagerService>(configuration);
        auto infoNodeResponse = clusterManagerService->getInfoNode();

        replication_t replication = std::make_shared<Replication>(configuration, clusterManagerService, infoNodeResponse);
        replication->setBooting();
        replication->initialize();

        return replication;
    }
};