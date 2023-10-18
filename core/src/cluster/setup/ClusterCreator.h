#pragma once

#include "shared.h"
#include "cluster/Cluster.h"
#include "logging/Logger.h"
#include "cluster/setup/ClusterNodeSetup.h"
#include "cluster/setup/SimpleClusterNodeSetup.h"
#include "cluster/setup/PartitionsClusterNodeSetup.h"

class ClusterCreator {
public:
    static cluster_t setup(configuration_t configuration, logger_t logger, onGoingSyncOplogs_t multipleResponses, memDbStores_t memDbStores);

    static clusterNodeSetup_t getClusterNodeSetupObject(configuration_t configuration, logger_t logger,
                                                        onGoingSyncOplogs_t multipleResponses, memDbStores_t memDbStores);
};