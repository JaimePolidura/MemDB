#pragma once

#include "cluster/clustermanager/responses/AllNodesResponse.h"
#include "cluster/clustermanager/responses/RingEntriesResponse.h"

#include "config/Configuration.h"
#include "utils/net/HttpClient.h"
#include "cluster/Node.h"
#include "logging/Logger.h"

#include "shared.h"

class ClusterManagerService {
    logger_t logger;
    HttpClient httpClusterManagerClient;
    configuration_t configuartion;
    std::string token;

public:
    ClusterManagerService(configuration_t configuartion, logger_t logger): configuartion(configuartion), logger(logger),
        token(""), httpClusterManagerClient() {}

    RingEntriesResponse getRingEntries() {
        this->token = this->authenticate();

        HttpResponse response = this->httpClusterManagerClient.get(
                this->configuartion->get(ConfigurationKeys::MEMDB_CORE_CLUSTER_MANAGER_ADDRESS),
                "/api/partitions/ring",
                this->token);

        if(!response.isSuccessful())
            throw std::runtime_error("Unexpected error when trying to get the ring from the cluster manager " + response.body.dump());

        return RingEntriesResponse::fromJson(response.body);
    }

    AllNodesResponse getAllNodes() {
        this->token = this->authenticate();

        HttpResponse response = this->httpClusterManagerClient.get(
                this->configuartion->get(ConfigurationKeys::MEMDB_CORE_CLUSTER_MANAGER_ADDRESS),
                "/api/nodes/all",
                this->token);

        if(!response.isSuccessful())
            throw std::runtime_error("Unexpected error when trying to get all nodes from the cluster manager " + response.body.dump());

        return AllNodesResponse::fromJson(response.body);
    }

private:
    std::string authenticate() {
        auto response = this->httpClusterManagerClient.post(
                this->configuartion->get(ConfigurationKeys::MEMDB_CORE_CLUSTER_MANAGER_ADDRESS),
                "/login",
                {{"authKey", this->configuartion->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY)}});

        if (response.code == 403) {
            logger->error("Invalid cluster auth key while trying to authenticate to they clustermanager");
            exit(-1);
        }

        return response.body["token"];
    }
};

using clusterManagerService_t = std::shared_ptr<ClusterManagerService>;