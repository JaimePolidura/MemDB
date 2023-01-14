#include "Map.h"

Map::Map(uint16_t numberBuckets): numberBuckets(numberBuckets) {
    buckets.reserve(numberBuckets);

    for (int i = 0; i < numberBuckets; i++) {
        buckets.emplace_back();
        locks.push_back(new boost::shared_mutex());
    }
}

void Map::put(const std::string &key, uint8_t * value, size_t valueSize) {
    uint32_t keyHash = this->calculateHash(key);

    lockWrite(keyHash);

    AVLTree * bucket = this->getBucket(keyHash);
    if(bucket->add(keyHash, value, valueSize))
        this->size++;

    unlockWrite(keyHash);
}

std::vector<MapEntry> Map::all() {
    std::vector<MapEntry> all{};

    for (const AVLTree bucket: this->buckets)
        for (const auto node : bucket.all())
            all.push_back(MapEntry{node->keyHash, node->value, node->valueLength});

    return all;
}

std::optional<MapEntry> Map::get(const std::string &key) const {
    uint32_t hash = this->calculateHash(key);

    lockRead(hash);

    AVLNode * nodeFoundForKey = this->getNodeByKeyHash(hash);
    const std::optional<MapEntry> response = nodeFoundForKey != nullptr ?
            std::optional<MapEntry>{MapEntry{nodeFoundForKey->keyHash, nodeFoundForKey->value, nodeFoundForKey->valueLength}} :
            std::nullopt;

    unlockRead(hash);

    return response;
}

void Map::remove(const std::string &key) {
    uint32_t hash = this->calculateHash(key);

    lockWrite(hash);

    AVLTree * bucket = this->getBucket(hash);
    if(bucket->contains(hash)) {
        bucket->remove(hash);
        this->size--;
    }

    unlockWrite(hash);
}

bool Map::contains(const std::string &key) const {
    return this->getNodeByKeyHash(this->calculateHash(key)) != nullptr;
}

int Map::getSize() const {
    return this->size;
}