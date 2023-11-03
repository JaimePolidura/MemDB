#include "MemDbCreator.h"

std::shared_ptr<MemDb> MemDbCreator::create(int nArgs, char ** args) {
    configuration_t configuration = ConfiguartionLoader::load(nArgs, args);
    logger_t logger = std::make_shared<Logger>(configuration, "Starting memdb");
    onGoingSyncOplogs_t syncOplogsStore = std::make_shared<OnGoingSyncOplogsStore>(configuration->get<memdbNodeId_t >(ConfigurationKeys::NODE_ID));
    memDbStores_t memDbStores = std::make_shared<MemDbStores>();

    //TODO Fix. If I dont put this, DelayedOperationBuffer will segfault at initialization
    std::queue<Request> req{};

    cluster_t cluster = createClusterObject(logger, configuration, syncOplogsStore, memDbStores);
    memDbStores->initializeStoresMap(cluster->getNodesPerPartition(), configuration);

    operationLog_t operationLog = createOperationLogObject(configuration, cluster, logger);

    lamportClock_t clock = std::make_shared<LamportClock>(1);
    operatorDispatcher_t operatorDispatcher = std::make_shared<OperatorDispatcher>(memDbStores, clock, cluster, configuration, logger, operationLog, syncOplogsStore);
    tcpServer_t tcpServer = std::make_shared<TCPServer>(logger, configuration, Authenticator{configuration}, operatorDispatcher);

    setupClusterChangeWatcher(cluster, operationLog, configuration, logger, operatorDispatcher, syncOplogsStore, memDbStores);

    return std::make_shared<MemDb>(logger, memDbStores, configuration, operatorDispatcher, tcpServer, clock, cluster, operationLog);
}

operationLog_t MemDbCreator::createOperationLogObject(configuration_t configuration, cluster_t cluster, logger_t logger) {
    if(configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)){
        return setupMultipleOplogConfiguration(configuration, cluster, logger);
    }else{
        return std::make_shared<SingleOperationLog>(configuration, 0, logger);
    }
}

cluster_t MemDbCreator::createClusterObject(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t multipleResponses, memDbStores_t memDbStores) {
    if(configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
        return ClusterCreator::setup(configuration, logger, multipleResponses, memDbStores);
    }else{
        return std::make_shared<Cluster>(configuration);
    }
}

operationLog_t MemDbCreator::setupMultipleOplogConfiguration(configuration_t configuration, cluster_t cluster, logger_t logger) {
    auto fileNameResolver = [](int iterations) -> std::string{
        return "oplog-" + iterations;
    };

    return std::make_shared<MultipleOperationLog>(configuration, fileNameResolver, cluster->getPartitionObject()->getNodesPerPartition(), logger);
}

void MemDbCreator::setupClusterChangeWatcher(cluster_t cluster, operationLog_t operationLog, configuration_t configuration,
                                             logger_t logger, operatorDispatcher_t operatorDispatcher, onGoingSyncOplogs_t multipleResponses, memDbStores_t memDbStores) {
    if(!configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
        return;
    }

    auto setupObject = ClusterCreator::getClusterNodeSetupObject(configuration, logger, multipleResponses, memDbStores);
    auto clusterChangeHandler = setupObject->getClusterDbChangeNodeHandler(cluster, operationLog, operatorDispatcher);

    cluster->watchForChangesInNodesClusterDb([clusterChangeHandler](node_t changedNode, ClusterDbChangeType type) -> void {
        clusterChangeHandler->handleChange(changedNode, type);
    });
}