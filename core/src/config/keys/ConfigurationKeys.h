#pragma once

#include "shared.h"

class ConfigurationKeys {
public:
    static const std::string MEMDB_CORE_AUTH_NODE_KEY;
    static const std::string MEMDB_CORE_AUTH_MAINTENANCE_KEY;
    static const std::string MEMDB_CORE_AUTH_API_KEY;

    static const std::string MEMDB_CORE_DATA_PATH;
    static const std::string MEMDB_CORE_PORT;
    static const std::string MEMDB_CORE_SERVER_THREADS;
    static const std::string MEMDB_CORE_NUMBER_BUCKETS;
    static const std::string MEMDB_CORE_PERSISTANCE_WRITE_EVERY;

    static const std::string MEMDB_CORE_SHOW_DEBUG_LOG;
    static const std::string MEMDB_CORE_USE_REPLICATION;
    static const std::string MEMDB_CORE_CLUSTER_MANAGER_ADDRESS;
    static const std::string MEMDB_CORE_ETCD_ADDRESSES;
    static const std::string MEMDB_CORE_NODE_ID;
    static const std::string MEMDB_CORE_USE_PARTITIONS;
};