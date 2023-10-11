#include "cluster/setup/ClusterCreator.h"

cluster_t ClusterCreator::setup(configuration_t configuration, logger_t logger, onGoingMultipleResponsesStore_t multipleResponses, memDbStores_t memDbStores)  {
    clusterNodeSetup_t setup = getClusterNodeSetupObject(configuration, logger, multipleResponses, memDbStores);
    cluster_t cluster = setup->create();
    setup->initializeNodeInCluster(cluster);

    return cluster;
}

clusterNodeSetup_t ClusterCreator::getClusterNodeSetupObject(configuration_t configuration, logger_t logger,
                                                             onGoingMultipleResponsesStore_t multipleResponses, memDbStores_t memDbStores) {
    if(!configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
        return std::make_shared<SimpleClusterNodeSetup>(logger, configuration, multipleResponses);
    }else{
        return std::make_shared<PartitionsClusterNodeSetup>(logger, configuration, multipleResponses, memDbStores);
    }
}