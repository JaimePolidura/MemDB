#pragma once

#include <string>
#include "HashCreator.hpp"

//TODO Replace with union
class MapValueEntry {
private:
    void * value;
    size_t valueSize;
    bool found;

public:
    MapValueEntry(void * valueCons, size_t valueSizeCons): value(valueCons), valueSize(valueSizeCons), found{true} {}

    MapValueEntry(): found{false} {}

    bool isFound() const {
        return this->found;
    }

    void * getValue() const {
        return this->value;
    }

    size_t getSize() const {
        return this->valueSize;
    }
};

class MapNode {
private:
    MapNode * next;
    int keyHash;
    void * data;
    size_t size;
    bool deleted;

public:
    MapNode(void * newData, int newKeyHash, size_t newSize): keyHash{newKeyHash}, data{newData}, size{newSize}, next{nullptr}, deleted{false} {}

    void replaceDeletedNode(int newHash, void * newData, size_t newSize) {
        this->data = newData;
        this->keyHash = newHash;
        this->size = newSize;
        this->deleted = false;
    }

    void replaceValue(void * newData, size_t newSize) {
        this->data = newData;
        this->size = newSize;
    }

    void setDeleted() {
        this->deleted = true;
        this->deleteDataPointer();
    }

    void setNext(MapNode * newNext) {
        this->next = newNext;
    }

    void * getData() const {
        return this->data;
    }

    size_t getSize() const {
        return this->size;
    }

    MapNode * getNext() const {
        return this->next;
    }

    bool isNotDeleted() const {
        return !this->deleted;
    }

    bool isDeleted() const {
        return this->deleted;
    }

    bool hasNotSameHash(int otherHashKey) const {
        return this->keyHash != otherHashKey;
    }

    bool hasSameHash(int otherKeyHash) const {
        return this->keyHash == otherKeyHash;
    }

    bool hasNoNext() const {
        return this->next == nullptr;
    }

    bool hasNext() const {
        return this->next != nullptr;
    }

private:
    void deleteDataPointer() {
        auto dataPointerCastedToChar = (char *) this->data;

        for(int i = 0; i < this->size; i++)
            delete (dataPointerCastedToChar + i);
    }
};

class Map{
private:
    static const int NUMBER_OF_BUCKETS = 1;

    HashCreator<std::string> * hashCreator;
    MapNode * buckets[NUMBER_OF_BUCKETS];
    int size;

public:
    Map(HashCreator<std::string> * hashCreatorCons): hashCreator{hashCreatorCons}, size{0} {}

    void put(const std::string& key, void * value, size_t valueSize) {
        int keyHash = this->hashCreator->create(key);
        MapNode * actualMapNode = this->getBucket(keyHash);

        while (actualMapNode->hasNext() || actualMapNode->isNotDeleted() || actualMapNode->hasNotSameHash(keyHash))
            actualMapNode = actualMapNode->getNext();

        if(actualMapNode->hasNoNext() && actualMapNode->isNotDeleted() && actualMapNode->hasNotSameHash(keyHash))
            actualMapNode->setNext( new MapNode(value, keyHash, valueSize));
        else if(actualMapNode->isDeleted())
            actualMapNode->replaceDeletedNode(keyHash, value, valueSize);
        else if(actualMapNode->hasSameHash(keyHash))
            actualMapNode->replaceValue(value, valueSize);
    }

    MapValueEntry get(const std::string &key) {
        MapNode * nodeFoundForKey = this->getNodeByKeyHash(this->hashCreator->create(key));

        return nodeFoundForKey != nullptr ?
               MapValueEntry{nodeFoundForKey->getData(), nodeFoundForKey->getSize()} :
               MapValueEntry{}; //Not found;
    }

    bool remove(const std::string &key) {
        MapNode * nodeFoundForKey = this->getNodeByKeyHash(this->hashCreator->create(key));

        if(nodeFoundForKey)
            nodeFoundForKey->setDeleted();

        return nodeFoundForKey;
    }

    bool contains(const std::string &key) {
        return this->getNodeByKeyHash(this->hashCreator->create(key)) != nullptr;
    }

    int getSize() {
        return this->size;
    }

private:
    MapNode * getNodeByKeyHash(int keyHash) {
        MapNode * actualMapNode = this->getBucket(keyHash);

        while (actualMapNode->hasNext() || actualMapNode->isNotDeleted())
            if(actualMapNode->hasSameHash(keyHash))
                return actualMapNode;

        return actualMapNode->hasSameHash(keyHash) ? actualMapNode : nullptr;
    }

    MapNode * getBucket(int keyHash) {
        int bucketSlot = keyHash % NUMBER_OF_BUCKETS;
        return this->buckets[bucketSlot];
    }
};
