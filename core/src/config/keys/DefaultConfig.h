#pragma once

#include "shared.h"

#include "config/keys/ConfigurationKeys.h"

class DefaultConfig {
private:
#ifdef _WIN32
#define FILEPATH "C:\\memdb"
#else
#define FILEPATH "/etc/memdb"
#endif

public:
    static std::map<std::string, std::string> get() {
        return {
                {ConfigurationKeys::AUTH_NODE_KEY,                      "123",},
                {ConfigurationKeys::MAINTENANCE_KEY,                    "456",},
                {ConfigurationKeys::AUTH_API_KEY,                       "789",},
                {ConfigurationKeys::SERVER_THREADS,                     "64"},
                {ConfigurationKeys::SERVER_PORT,                        "10000"},
                {ConfigurationKeys::NUMBER_BUCKETS,                     "64"},
                {ConfigurationKeys::PERSISTENCE_SEGMENT_SIZE_MB,        "100"},
                {ConfigurationKeys::USE_REPLICATION,                    "false"},
                {ConfigurationKeys::ETCD_ADDRESSES,                     "127.0.0.1:2379"},
                {ConfigurationKeys::CLUSTER_MANAGER_ADDRESS,            "127.0.0.1:8080"},
                {ConfigurationKeys::NODE_ID,                            "1"},
                {ConfigurationKeys::SHOW_DEBUG_LOG,                     "false"},
                {ConfigurationKeys::USE_PARTITIONS,                     "false"},
                {ConfigurationKeys::DATA_PATH,                          FILEPATH}
        };
    }
};