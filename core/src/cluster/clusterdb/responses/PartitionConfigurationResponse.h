#pragma once

#include "shared.h"

struct PartitionConfigurationResponse {
    uint32_t nodesPerPartition;
    uint32_t maxSize;
};