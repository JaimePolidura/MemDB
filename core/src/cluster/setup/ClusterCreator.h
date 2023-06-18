#pragma once

#include "shared.h"
#include "cluster/clustermanager/responses/AllNodesResponse.h"
#include "cluster/Cluster.h"
#include "logging/Logger.h"
#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/setup/SimpleClusterNodeSetup.h"
#include "cluster/setup/PartitionsClusterNodeSetup.h"

class ClusterCreator {
public:
    static auto setup(configuration_t configuration, logger_t logger) -> cluster_t {
        clusterNodeSetup_t setup = getClusterNodeSetupObject(configuration, logger);
        cluster_t cluster = setup->create();
        setup->initializeNodeInCluster(cluster);

        return cluster;
    }

    static auto getClusterNodeSetupObject(configuration_t configuration, logger_t logger) -> clusterNodeSetup_t {
        if(!configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
            return std::make_shared<SimpleClusterNodeSetup>(logger, configuration);
        }else{
            return std::make_shared<PartitionsClusterNodeSetup>(logger, configuration);
        }
    }
};