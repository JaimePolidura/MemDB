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
    printf("RUN SET ON %s,", key.data());

    AVLTree * bucket = this->getBucket(keyHash);
    bucket->add(keyHash, value, valueSize);

    unlockWrite(keyHash);

    this->size++;
}

std::optional<MapEntry> Map::get(const std::string &key) const {
    uint32_t hash = this->calculateHash(key);

    lockRead(hash);

    AVLNode * nodeFoundForKey = this->getNodeByKeyHash(hash);
    const std::optional<MapEntry> response = nodeFoundForKey != nullptr ?
            std::optional<MapEntry>{MapEntry{nodeFoundForKey->value, nodeFoundForKey->valueLength}} :
            std::nullopt;

    unlockRead(hash);

    return response;
}

void Map::remove(const std::string &key) {
    uint32_t hash = this->calculateHash(key);

    lockWrite(hash);
    printf("RUN REMOVE ON %s,", key.data());
    AVLNode * nodeFoundForKey = this->getNodeByKeyHash(hash);

    if(nodeFoundForKey != nullptr) {
        AVLTree * bucket = this->getBucket(hash);
        bucket->remove(hash);
    }

    unlockWrite(hash);

    this->size--;
}

bool Map::contains(const std::string &key) const {
    return this->getNodeByKeyHash(this->calculateHash(key)) != nullptr;
}

int Map::getSize() const {
    return this->size;
}