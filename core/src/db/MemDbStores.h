#pragma once

#include "shared.h"

#include "db/Map.h"
#include "config/Configuration.h"
#include "Map.h"

class MemDbStores {
private:
    std::map<uint32_t, memDbDataStoreMap_t> storesByPartitionId{};

public:
    MemDbStores() = default;

    void initializeStoresMap(uint32_t partitionsPerNode, configuration_t configuration);

    memDbDataStoreMap_t getByPartitionId(uint32_t partitionId);
};

using memDbStores_t = std::shared_ptr<MemDbStores>;