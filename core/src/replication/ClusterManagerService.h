#pragma once

#include "config/Configuration.h"

#include <memory>

class ClusterManagerService {
    std::shared_ptr<Configuration> configuartion;

public:
    ClusterManagerService(std::shared_ptr<Configuration> configuartion): configuartion(configuartion) {}

    int32_t getNodeId() {
        //MakeHttpRequest to clustermanager
        return 1;
    }
};