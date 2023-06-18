#pragma once

#include "cluster/clustermanager/responses/AllNodesResponse.h"
#include "cluster/clustermanager/responses/GetRingInfoResponse.h"
#include "cluster/clustermanager/responses/GetRingNeighborsResponse.h"

#include "config/Configuration.h"
#include "utils/net/HttpClient.h"
#include "cluster/Node.h"
#include "logging/Logger.h"

#include "shared.h"

class ClusterManagerService {
    logger_t logger;
    HttpClient httpClusterManagerClient;
    configuration_t configuration;
    std::string token;

public:
    ClusterManagerService(configuration_t configuartion, logger_t logger): configuration(configuartion), logger(logger),
                                                                           token(""), httpClusterManagerClient() {}

    GetRingInfoResponse getRingInfo() {
        this->token = this->authenticate();

        HttpResponse response = this->httpClusterManagerClient.get(
                this->configuration->get(ConfigurationKeys::MEMDB_CORE_CLUSTER_MANAGER_ADDRESS),
                "/api/partitions/ring/info",
                this->token);

        if(!response.isSuccessful())
            throw std::runtime_error("Unexpected error when trying to get the ring from the cluster manager " + response.body.dump());

        return GetRingInfoResponse::fromJson(response.body);
    }

    AllNodesResponse getAllNodes(memdbNodeId_t nodeId) {
        this->token = this->authenticate();

        HttpResponse response = this->httpClusterManagerClient.get(
                this->configuration->get(ConfigurationKeys::MEMDB_CORE_CLUSTER_MANAGER_ADDRESS),
                "/api/nodes/all?nodeId=" + std::to_string(nodeId),
                this->token);

        if(!response.isSuccessful())
            throw std::runtime_error("Unexpected error when trying to get all nodes from the cluster manager " + response.body.dump());

        return AllNodesResponse::fromJson(response.body);
    }

private:
    std::string authenticate() {
        auto response = this->httpClusterManagerClient.post(
                this->configuration->get(ConfigurationKeys::MEMDB_CORE_CLUSTER_MANAGER_ADDRESS),
                "/login",
                {{"authKey", this->configuration->get(ConfigurationKeys::MEMDB_CORE_AUTH_API_KEY)}});

        if (response.code == 403) {
            logger->error("Invalid cluster auth key while trying to authenticate to they clustermanager");
            exit(-1);
        }

        return response.body["token"];
    }
};

using clusterManagerService_t = std::shared_ptr<ClusterManagerService>;