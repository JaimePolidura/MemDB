#include "MemDbCreator.h"

std::shared_ptr<MemDb> MemDbCreator::create(int nArgs, char ** args) {
    configuration_t configuration = ConfiguartionLoader::load(nArgs, args);
    logger_t logger = std::make_shared<Logger>(configuration, "Starting memdb");
    onGoingSyncOplogs_t syncOplogsStore = std::make_shared<OnGoingSyncOplogsStore>(configuration->get<memdbNodeId_t >(ConfigurationKeys::NODE_ID));
    memDbStores_t memDbStores = std::make_shared<MemDbStores>();
    lamportClock_t clock = std::make_shared<LamportClock>(1);
    operationLog_t operationLog = createOperationLogObject(configuration, logger);

    //TODO Fix. If I dont put this, DelayedOperationBuffer will segfault at initialization
    std::queue<Request> req{};

    operatorDispatcher_t operatorDispatcher = std::make_shared<OperatorDispatcher>();

    cluster_t cluster = createClusterObject(logger, configuration, syncOplogsStore, memDbStores, operatorDispatcher, operationLog);
    memDbStores->initializeStoresMap(cluster->getNodesPerPartition(), configuration);

    tcpServer_t tcpServer = std::make_shared<TCPServer>(logger, configuration, Authenticator{configuration}, operatorDispatcher);

    operatorDispatcher->logger = logger;
    operatorDispatcher->configuration = configuration;
    operatorDispatcher->clock = clock;
    operatorDispatcher->cluster = cluster;
    operatorDispatcher->operationLog = operationLog;
    operatorDispatcher->onGoingSyncOplogs = syncOplogsStore;

    initializeOplog(configuration, operationLog, cluster);

    return std::make_shared<MemDb>(logger, memDbStores, configuration, operatorDispatcher, tcpServer, clock, cluster, operationLog);
}

operationLog_t MemDbCreator::createOperationLogObject(configuration_t configuration, logger_t logger) {
    if(configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)){
        return std::make_shared<MultipleOperationLog>(configuration, logger, [](int iterations) -> std::string{
            return "oplog-" + iterations;
        });
    } else {
        return std::make_shared<SingleOperationLog>(configuration, logger);
    }
}

cluster_t MemDbCreator::createClusterObject(logger_t logger, configuration_t configuration, onGoingSyncOplogs_t multipleResponses,
    memDbStores_t memDbStores, operatorDispatcher_t operatorDispatcher, operationLog_t operationLog) {

    if(configuration->getBoolean(ConfigurationKeys::USE_REPLICATION)){
        cluster_t cluster = std::make_shared<Cluster>(logger, configuration, multipleResponses, memDbStores);

        clusterNodeSetup_t setup;
        if(configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS))
            setup = std::make_shared<PartitionsClusterNodeSetup>(operatorDispatcher, multipleResponses, configuration, operationLog, memDbStores, cluster, logger);
        else
            setup = std::make_shared<SimpleClusterNodeSetup>(operatorDispatcher, multipleResponses, configuration, operationLog, memDbStores, cluster, logger);

        setup->initializeNodeInCluster();

        return cluster;
    } else {
        return std::make_shared<Cluster>(configuration);
    }
}

void MemDbCreator::initializeOplog(configuration_t configuration, operationLog_t operationLog, cluster_t cluster) {
    if(configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS)) {
        std::dynamic_pointer_cast<MultipleOperationLog>(operationLog)->initializeOplogs(cluster->getNodesPerPartition());
    } else {
        std::dynamic_pointer_cast<SingleOperationLog>(operationLog)->initialize(0);
    }
}