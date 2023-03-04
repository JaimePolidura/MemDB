#pragma once

#include <string>

class ConfigurationKeys {
public:
    static const std::string AUTH_USER_KEY;
    static const std::string PORT;
    static const std::string SERVER_MIN_THREADS;
    static const std::string SERVER_MAX_THREADS;
    static const std::string NUMBER_BUCKETS;
    static const std::string PERSISTANCE_WRITE_EVERY;

    static const std::string USE_REPLICATION;
    static const std::string CLUSTER_MANAGER_ADDRESS;
    static const std::string CLUSTER_MANAGER_ADDRESS_USING_DNS;
    static const std::string AUTH_CLUSTER_KEY;
    static const std::string ETCD_ADDRESSES;
};