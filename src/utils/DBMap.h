#pragma once

#include "Map.h"
#include "optional"
#include "HashCreator.h"

class Node {
private:
    Node * next;
    int keyHash;
    char * value;
    size_t valueSize;
    bool deleted;

public:
    Node(char * newData, int newKeyHash, size_t newSize): keyHash{newKeyHash}, value{newData}, valueSize{newSize}, next{nullptr}, deleted{false} {}

    void replaceDeletedNode(int newHash, char * newData, size_t newSize) {
        this->value = newData;
        this->keyHash = newHash;
        this->valueSize = newSize;
        this->deleted = false;
    }

    void replaceValue(char * newData, size_t newSize) {
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
        for(int i = 0; i < this->valueSize; i++)
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

    void put(const std::string& key, char * value, size_t valueSize) override;

    std::optional<MapEntry> get(const std::string &key) override;

    std::optional<MapEntry> remove(const std::string &key) override;

    bool contains(const std::string &key) override;

    int getSize() override;

private:
    Node * getNodeByKeyHash(int keyHash) {
        Node * actualMapNode = this->getBucket(keyHash);

        while (actualMapNode != nullptr && (actualMapNode->hasNext() || actualMapNode->isNotDeleted()))
            if(actualMapNode->hasSameHash(keyHash))
                return actualMapNode;

        return (actualMapNode != nullptr && actualMapNode->hasSameHash(keyHash)) ?
            actualMapNode :
            nullptr;
    }

    int getBucketSlot(int keyHash) {
        return keyHash % NUMBER_OF_BUCKETS;
    }

    Node * getBucket(int keyHash) {
        return this->buckets[this->getBucketSlot(keyHash)];
    }
};