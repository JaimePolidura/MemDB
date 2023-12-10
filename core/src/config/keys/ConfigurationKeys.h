#pragma once

#include "shared.h"

class ConfigurationKeys {
public:
    static const std::string AUTH_NODE_KEY;
    static const std::string AUTH_API_USER_KEY;

    static const std::string DATA_PATH;
    static const std::string SERVER_PORT;
    static const std::string SERVER_THREADS;
    static const std::string NUMBER_BUCKETS;
    static const std::string PERSISTENCE_SEGMENT_SIZE_MB;

    static const std::string SHOW_DEBUG_LOG;
    static const std::string USE_REPLICATION;
    static const std::string NODE_ID;
    static const std::string ADDRESS;
    static const std::string USE_PARTITIONS;
    static const std::string NODE_REQUEST_TIMEOUT_MS;
    static const std::string NODE_REQUEST_N_RETRIES;
    static const std::string SEED_NODES;

    static const std::string NODES_PER_PARTITION;
    static const std::string MAX_PARTITION_SIZE;
};