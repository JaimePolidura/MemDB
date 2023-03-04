#pragma once

#include "replication/clustermanager/requests/SetupNodeResponse.h"
#include "config/Configuration.h"
#include "utils/http/HttpClient.h"
#include "replication/Node.h"

#include <nlohmann/json.hpp>
#include <memory>

class ClusterManagerService {
    std::shared_ptr<Configuration> configuartion;
    std::string token;

public:
    ClusterManagerService() = default;

    ClusterManagerService(std::shared_ptr<Configuration> configuartion): configuartion(configuartion), token("") {}

    SetupNodeResponse setupNode() {
        this->token = this->authenticate();

        std::string url =  + "";
        HttpResponse response = HttpClient::get(
                this->configuartion->get(ConfigurationKeys::CLUSTER_MANAGER_ADDRESS),
                "/api/nodes/selfinfo",
                this->token);

        if(!response.isSuccessful())
            throw std::runtime_error("Cluster manager not found");

        return SetupNodeResponse::fromJson(response.body);
    }

private:
    std::string authenticate() {
        auto response = HttpClient::post(
                this->configuartion->get(ConfigurationKeys::CLUSTER_MANAGER_ADDRESS),
                "/login",
                {{"authKey", this->configuartion->get(ConfigurationKeys::AUTH_CLUSTER_KEY)}});

        if (response.code == 403) {
            printf("[SERVER] Invalid auth key");
            exit(-1);
        }

        return response.body["token"];
    }
};

using clusterManagerService_t = std::shared_ptr<ClusterManagerService>;