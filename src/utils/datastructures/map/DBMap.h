#pragma once

#include "Map.h"
#include "optional"
#include "utils/crypto/HashCreator.h"

class Node {
private:
    Node * next;
    int keyHash;
    uint8_t * value;
    size_t valueSize;
    bool deleted;

public:
    Node(uint8_t * newData, int newKeyHash, size_t newSize): keyHash{newKeyHash}, value{newData}, valueSize{newSize}, next{nullptr}, deleted{false} {}

    void replaceDeletedNode(int newHash, uint8_t * newData, size_t newSize) {
        this->value = newData;
        this->keyHash = newHash;
        this->valueSize = newSize;
        this->deleted = false;
    }

    void replaceValue(uint8_t * newData, size_t newSize) {
        this->value = newData;
        this->valueSize = newSize;
    }

    void setDeleted() {
        this->deleted = true;
        this->deleteDataPointer();
    }

    void setNext(Node * newNext) {
        this->next = newNext;
    }

    MapEntry toEntry() const { return MapEntry{value, valueSize}; }
    Node * getNext() const { return this->next; }
    bool isNotDeleted() const { return !this->deleted; }
    bool isDeleted() const { return this->deleted; }
    bool hasNotSameHash(int otherHashKey) const { return this->keyHash != otherHashKey; }
    bool hasSameHash(int otherKeyHash) const { return this->keyHash == otherKeyHash; }
    bool hasNoNext() const { return this->next == nullptr; }
    bool hasNext() const { return this->next != nullptr; }

private:
    void deleteDataPointer() {
        for(int i = 0; i <= this->valueSize / sizeof(this->value); i++)
            delete (this->value + i);
    }
};

class DBMap : public Map {
private:
    static const int NUMBER_OF_BUCKETS = 64;

    HashCreator<std::string> * hashCreator;
    Node * buckets[NUMBER_OF_BUCKETS] = {};
    int size;

public:
    DBMap(HashCreator<std::string> * hashCreator);

    void put(const std::string& key, uint8_t * value, size_t valueSize) override;

    std::optional<MapEntry> get(const std::string &key) const override;

    std::optional<MapEntry> remove(const std::string &key) override;

    bool contains(const std::string &key) const override;

    int getSize() const override;

private:
    Node * getNodeByKeyHash(int keyHash) const {
        Node * actualMapNode = this->getBucket(keyHash);

        while (actualMapNode != nullptr && (actualMapNode->hasNext() || actualMapNode->isNotDeleted()))
            if(actualMapNode->hasSameHash(keyHash) && actualMapNode->isNotDeleted())
                return actualMapNode;

        return (actualMapNode != nullptr && actualMapNode->hasSameHash(keyHash) && actualMapNode->isNotDeleted()) ?
            actualMapNode :
            nullptr;
    }

    Node * getBucket(int keyHash) const {
        return this->buckets[this->getBucketSlot(keyHash)];
    }

    int getBucketSlot(int keyHash) const{
        return keyHash % NUMBER_OF_BUCKETS;
    }
};