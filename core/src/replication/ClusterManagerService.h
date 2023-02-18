#pragma once

#include "config/Configuration.h"
#include "Node.h"

#include <memory>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>


#include "utils/http/HttpClient.h"

struct SetupNodeResponse {
    std::vector<Node> nodes;
    int nodeId;
};

class ClusterManagerService {
    std::shared_ptr<Configuration> configuartion;
    std::string token;

public:
    ClusterManagerService(std::shared_ptr<Configuration> configuartion): configuartion(configuartion), token("") {}

    SetupNodeResponse setupNode() {
        this->token = this->authenticate();

        std::string url = this->configuartion->get(ConfigurationKeys::CLUSTER_MANAGER_ADDRESS) + "/api/nodes/setup";
        HttpResponse response = HttpClient::post(url, {}, this->authenticate());

        if(!response.isSuccessful())
            throw std::runtime_error("Cluster manager not found");

        std::vector<Node> allNodes;
        auto jsonNodes = response.body["nodes"];
        for (const auto& nodeJson : jsonNodes) {
            allNodes.push_back(Node{
                    .address = nodeJson["address"].get<std::string>(),
                    .state = parseNodeStateFromString(nodeJson["state"].get<std::string>()),
                    .nodeId = nodeJson["nodeId"].get<int>(),
            });
        }

        return SetupNodeResponse{
                .nodes = allNodes,
                .nodeId = response.body["nodeId"].get<int>(),
        };
    }

    std::string authenticate() {
        auto response =HttpClient::post(
                this->configuartion->get(ConfigurationKeys::CLUSTER_MANAGER_ADDRESS) + "/api/login",
                {{"authKey", this->configuartion->get(ConfigurationKeys::AUTH_CLUSTER_KEY)}}
        );

        if (response.code == 403) {
            printf("[SERVER] Invalid auth key");
            exit(1);
        }

        return response.body["token"];
    }
};