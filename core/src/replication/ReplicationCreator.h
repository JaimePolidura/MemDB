#pragma once

#include <cstdint>
#include "replication/Replication.h"

class ReplicationCreator {
public:
    static auto setup(configuration_t configuration) -> replication_t {
        clusterManagerService_t clusterManagerService = std::make_shared<ClusterManagerService>(configuration);
        auto setupResponse = clusterManagerService->setupNode();

        replication_t replication = std::make_shared<Replication>(configuration, clusterManagerService, setupResponse);
        replication->initializeNodeConnections();
        replication->watchForChangesInClusterDb();

        return replication;
    }
};