#pragma once

#include "shared.h"
#include "cluster/clustermanager/responses/AllNodesResponse.h"
#include "cluster/Cluster.h"
#include "logging/Logger.h"

class ClusterCreator {
public:
    static auto setup(configuration_t configuration, logger_t logger) -> cluster_t {
        cluster_t cluster = std::make_shared<Cluster>(logger, configuration);
        cluster->setup(true);

        return cluster;
    }
};