#include "cluster/clustermanager/ClusterManagerService.h"

GetRingInfoResponse ClusterManagerService::getRingInfo() {
    this->token = this->authenticate();

    HttpResponse response = this->httpClusterManagerClient.get(
            this->configuration->get(ConfigurationKeys::CLUSTER_MANAGER_ADDRESS),
            "/api/partitions/ring/info",
            this->token);

    if(!response.isSuccessful())
        throw std::runtime_error("Unexpected error when trying to getAll the ring from the cluster manager " + response.body.dump());

    return GetRingInfoResponse::fromJson(response.body);
}

AllNodesResponse ClusterManagerService::getAllNodes(memdbNodeId_t nodeId) {
    this->token = this->authenticate();

    HttpResponse response = this->httpClusterManagerClient.get(
            this->configuration->get(ConfigurationKeys::CLUSTER_MANAGER_ADDRESS),
            "/api/nodes/all?nodeId=" + std::to_string(nodeId),
            this->token);

    if(!response.isSuccessful())
        throw std::runtime_error("Unexpected error when trying to getAll all nodes from the cluster manager " + response.body.dump());

    return AllNodesResponse::fromJson(response.body);
}

std::string ClusterManagerService::authenticate() {
    auto response = this->httpClusterManagerClient.post(
            this->configuration->get(ConfigurationKeys::CLUSTER_MANAGER_ADDRESS),
            "/login",
            {{"authKey", this->configuration->get(ConfigurationKeys::AUTH_API_KEY)}});

    if (response.code == 403) {
        logger->error("Invalid cluster auth key while trying to authenticate to they clustermanager");
        exit(-1);
    }

    return response.body["token"];
}