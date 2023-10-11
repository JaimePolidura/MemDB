#include "MemDbStores.h"

void MemDbStores::initializeStoresMap(uint32_t partitionsPerNode, configuration_t configuration) {
    for(int i = 0; i < partitionsPerNode; i++){
        this->storesByPartitionId[i] = std::make_shared<Map<memDbDataLength_t>>(
                configuration->get<int>(ConfigurationKeys::MEMDB_CORE_NUMBER_BUCKETS)
        );
    }
}

memDbDataStoreMap_t MemDbStores::getByPartitionId(uint32_t partitionId) {
    return this->storesByPartitionId.at(partitionId);
}