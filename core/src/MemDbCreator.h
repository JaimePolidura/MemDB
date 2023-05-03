#pragma once

#include <memory>

#include "MemDb.h"
#include "config/keys/ConfigurationKeys.h"
#include "cluster/Cluster.h"
#include "auth/Authenticator.h"
#include "config/ConfigurationLoader.h"

#include "persistence/oplog/MultipleOperationLog.h"
#include "persistence/oplog/SingleOperationLog.h"

class MemDbCreator {
public:
    static std::shared_ptr<MemDb> create() {
        configuration_t configuration = ConfiguartionLoader::load();
        logger_t logger = std::make_shared<Logger>(configuration, "Starting memdb");

        cluster_t cluster = createClusterObject(logger, configuration);
        operationLog_t operationLog = createOperationLogObject(configuration, cluster);

        lamportClock_t clock = std::make_shared<LamportClock>(1);
        memDbDataStore_t map = std::make_shared<Map<memDbDataLength_t>>(configuration->get<uint16_t>(ConfigurationKeys::MEMDB_CORE_NUMBER_BUCKETS));
        operatorDispatcher_t operatorDispatcher = std::make_shared<OperatorDispatcher>(map, clock, cluster, configuration, logger, operationLog);
        tcpServer_t tcpServer = std::make_shared<TCPServer>(logger, configuration, Authenticator{configuration}, operatorDispatcher);

        setupClusterChangeWatcher(cluster, operationLog, configuration, logger, operatorDispatcher);

        return std::make_shared<MemDb>(logger, map, configuration, operatorDispatcher, tcpServer, clock, cluster, operationLog);
    }

private:
    static operationLog_t createOperationLogObject(configuration_t configuration, cluster_t cluster) {
        if(configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS)){
            return setupMultipleOplogConfiguration(configuration, cluster);
        }else{
            return std::make_shared<SingleOperationLog>(configuration, "oplog-0");
        }
    }

    static cluster_t createClusterObject(logger_t logger, configuration_t configuration) {
        if(configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION)){
            return ClusterCreator::setup(configuration, logger);
        }else{
            return std::make_shared<Cluster>();
        }
    }

    static operationLog_t setupMultipleOplogConfiguration(configuration_t configuration, cluster_t cluster) {
        auto fileNameResolver = [](int iterations) -> std::string{
            return "oplog-" + iterations;
        };
        auto oplogResolver = [cluster](const OperationBody& operationBody) -> int{
            return cluster->getPartitionObject()->getDistanceOfKey(operationBody.args->at(0));
        };

        return std::make_shared<MultipleOperationLog>(configuration, oplogResolver, fileNameResolver,
                                                      cluster->getPartitionObject()->getNodesPerPartition());
    }

    static void setupClusterChangeWatcher(cluster_t cluster, operationLog_t operationLog, configuration_t configuration, logger_t logger, operatorDispatcher_t operatorDispatcher) {
        if(!configuration->getBoolean(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION)){
            return;
        }

        auto setupObject = ClusterCreator::getClusterNodeSetupObject(configuration, logger);
        auto clusterChangeHandler = setupObject->getClusterDbChangeNodeHandler(cluster, operationLog, operatorDispatcher);

        cluster->watchForChangesInNodesClusterDb([clusterChangeHandler](node_t changedNode, ClusterDbChangeType type) -> void {
            clusterChangeHandler->handleChange(changedNode, type);
        });
    }
};