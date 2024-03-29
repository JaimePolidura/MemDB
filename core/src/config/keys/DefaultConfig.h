#pragma once

#include "shared.h"

#include "config/keys/ConfigurationKeys.h"

class DefaultConfig {
#ifdef _WIN32
#define FILEPATH "C:\\memdb"
#else
#define FILEPATH "/etc/memdb"
#endif

public:
    static std::map<std::string, std::string> get() {
        return {
                {ConfigurationKeys::AUTH_API_USER_KEY,                  "123"},
                {ConfigurationKeys::AUTH_NODE_KEY,                      "456"},
                {ConfigurationKeys::SERVER_THREADS,                     "63"},
                {ConfigurationKeys::SERVER_PORT,                        "10000"},
                {ConfigurationKeys::NUMBER_BUCKETS,                     "64"},
                {ConfigurationKeys::PERSISTENCE_SEGMENT_SIZE_MB,        "100"},
                {ConfigurationKeys::USE_REPLICATION,                    "false"},
                {ConfigurationKeys::NODE_ID,                            "1"},
                {ConfigurationKeys::SHOW_DEBUG_LOG,                     "false"},
                {ConfigurationKeys::USE_PARTITIONS,                     "false"},
                {ConfigurationKeys::NODE_REQUEST_TIMEOUT_MS,            "5000"},
                {ConfigurationKeys::NODE_REQUEST_N_RETRIES,             "10"},
                {ConfigurationKeys::SEED_NODES,                         ""},
                {ConfigurationKeys::ADDRESS,                            "localhost"},
                {ConfigurationKeys::NODES_PER_PARTITION,                "1"}, //Provided by seeder nodes
                {ConfigurationKeys::MAX_PARTITION_SIZE,                 "64"},
                {ConfigurationKeys::DATA_PATH,                          FILEPATH}
        };
    }
};