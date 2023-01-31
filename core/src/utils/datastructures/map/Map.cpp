#include "Map.h"

Map::Map(uint16_t numberBuckets): numberBuckets(numberBuckets) {
    buckets.reserve(numberBuckets);

    for (int i = 0; i < numberBuckets; i++) {
        buckets.emplace_back();
        locks.push_back(new boost::shared_mutex());
    }
}

bool Map::put(SimpleString& key, SimpleString& value, bool ignoreTimestamps, uint64_t timestamp, uint16_t nodeId) {
    uint32_t keyHash = this->calculateHash(key);

    lockWrite(keyHash);

    AVLTree * bucket = this->getBucket(keyHash);
    bool areadyContained = bucket->contains(keyHash);
    bool added = bucket->add(key, keyHash, value, ignoreTimestamps, timestamp, nodeId);

    if(added && !areadyContained) {
        this->size++;
    }

    unlockWrite(keyHash);

    return added;
}

std::vector<MapEntry> Map::all() {
    std::vector<MapEntry> all{};

    for (const AVLTree bucket: this->buckets){
        for (const auto node : bucket.all()){
            all.push_back(MapEntry{node->key, node->keyHash, node->value});
        }
    }

    return all;
}

std::optional<MapEntry> Map::get(SimpleString& key) const {
    uint32_t hash = this->calculateHash(key);

    lockRead(hash);

    AVLNode * node = this->getNodeByKeyHash(hash);

    const std::optional<MapEntry> response = (node != nullptr ?
            std::optional<MapEntry>{MapEntry{node->key, node->keyHash, node->value}} :
            std::nullopt);

    unlockRead(hash);

    return response;
}

bool Map::remove(SimpleString& key, bool ignoreTimestamps, uint64_t timestamp, uint16_t nodeId) {
    uint32_t hash = this->calculateHash(key);

    lockWrite(hash);

    AVLTree * bucket = this->getBucket(hash);
    bool removed = false;
    if(bucket->contains(hash) && (removed = bucket->remove(hash, ignoreTimestamps, timestamp, nodeId))) {
        this->size--;
    }

    unlockWrite(hash);

    return removed;
}

bool Map::contains(SimpleString& key) const {
    return this->getNodeByKeyHash(this->calculateHash(key)) != nullptr;
}

int Map::getSize() const {
    return this->size;
}