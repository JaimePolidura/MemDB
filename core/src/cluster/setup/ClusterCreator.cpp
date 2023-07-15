#include "cluster/setup/ClusterCreator.h"

cluster_t ClusterCreator::setup(configuration_t configuration, logger_t logger)  {
    clusterNodeSetup_t setup = getClusterNodeSetupObject(configuration, logger);
    cluster_t cluster = setup->create();
    setup->initializeNodeInCluster(cluster);

    return cluster;
}

clusterNodeSetup_t ClusterCreator::getClusterNodeSetupObject(configuration_t configuration, logger_t logger) {
    if(!configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
        return std::make_shared<SimpleClusterNodeSetup>(logger, configuration);
    }else{
        return std::make_shared<PartitionsClusterNodeSetup>(logger, configuration);
    }
}