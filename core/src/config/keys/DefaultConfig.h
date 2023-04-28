#pragma once

#include "shared.h"

#include "config/keys/ConfigurationKeys.h"

class DefaultConfig {
public:
    static std::map<std::string, std::string> get() {
        return {
                {ConfigurationKeys::MEMDB_CORE_AUTH_NODE_KEY,                     "123",},
                {ConfigurationKeys::MEMDB_CORE_AUTH_MAINTENANCE_KEY,              "456",},
                {ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY,                      "789",},
                {ConfigurationKeys::MEMDB_CORE_SERVER_MIN_THREADS,                "20"},
                {ConfigurationKeys::MEMDB_CORE_SERVER_MAX_THREADS,                "100"},
                {ConfigurationKeys::MEMDB_CORE_PORT,                              "10000"},
                {ConfigurationKeys::MEMDB_CORE_NUMBER_BUCKETS,                    "64" },
                {ConfigurationKeys::MEMDB_CORE_PERSISTANCE_WRITE_EVERY,           "50" },
                {ConfigurationKeys::MEMDB_CORE_USE_REPLICATION,                   "false"},
                {ConfigurationKeys::MEMDB_CORE_ETCD_ADDRESSES,                    "127.0.0.1:2379"},
                {ConfigurationKeys::MEMDB_CORE_CLUSTER_MANAGER_ADDRESS,           "127.0.0.1:8080"},
                {ConfigurationKeys::MEMDB_CORE_NODE_ID,                           "1"},
                {ConfigurationKeys::MEMDB_CORE_SHOW_DEBUG_LOG,                    "false"},
                {ConfigurationKeys::MEMDB_CORE_USE_PARTITIONS,                          "false"}
        };
    }
};