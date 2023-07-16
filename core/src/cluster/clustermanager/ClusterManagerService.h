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
                                                                           token(""), httpClusterManagerClient(logger) {}

    GetRingInfoResponse getRingInfo();

    AllNodesResponse getAllNodes(memdbNodeId_t nodeId);

private:
    std::string authenticate();
};

using clusterManagerService_t = std::shared_ptr<ClusterManagerService>;