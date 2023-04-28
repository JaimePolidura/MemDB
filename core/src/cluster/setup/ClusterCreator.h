#pragma once

#include "shared.h"
#include "cluster/clustermanager/responses/AllNodesResponse.h"
#include "cluster/Cluster.h"
#include "logging/Logger.h"
#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/setup/SimpleClusterNodeSetup.h"

class ClusterCreator {
public:
    static auto setup(configuration_t configuration, logger_t logger) -> cluster_t {
        clusterNodeSetup_t setup = getClusterNodeSetup(configuration, logger);
        cluster_t cluster = setup->create();
        setup->initializeNodeInCluster(cluster);

        return cluster;
    }

private:
    static auto getClusterNodeSetup(configuration_t configuration, logger_t logger) -> clusterNodeSetup_t {
        if(!configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
            return std::make_shared<SimpleClusterNodeSetup>(logger, configuration);
        }else{
            return ClusterNodeSetup{};
        }
    }
};