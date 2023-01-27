#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <vector>
#include <queue>

#include "utils/strings/SimpleString.h"
#include "utils/clock/LamportClock.h"

class AVLNode {
public:
    SimpleString key;
    SimpleString value;
    LamportClock timestamp;
    AVLNode * left;
    AVLNode * right;
    uint32_t keyHash;
    int16_t height;

    bool hasNoChild() {
        return this->left == nullptr && this->right == nullptr;
    }

    AVLNode(SimpleString key, uint32_t keyHash, SimpleString value, int16_t height, uint16_t nodeId, uint64_t timestamp):
            left(nullptr), right(nullptr), value(value), keyHash(keyHash), height(height), key(key), timestamp(nodeId, timestamp) {
    }
};

class AVLTree {
public:
    AVLNode * root;

public:
    bool add(const SimpleString& key, uint32_t keyHash, const SimpleString& value, uint64_t timestamp, uint16_t nodeId) {
        AVLNode * newNode = new AVLNode(key, keyHash, value, -1, nodeId, timestamp);

        if(this->root == nullptr){
            this->root = newNode;
            return true;
        }

        AVLNode * insertedNode = this->insertRecursive(this->root, newNode);
        return insertedNode != nullptr;
    }

    bool remove(uint32_t keyHash, uint64_t timestamp, uint16_t nodeId) {
        bool removed = false;
        this->removeRecursive(this->root, keyHash, timestamp, nodeId, removed);

        return removed;
    }

    std::vector<AVLNode *> all() const {
        std::vector<AVLNode *> toReturn{};
        if(this->root == nullptr) return toReturn;

        std::queue<AVLNode *> pending{};
        pending.push(this->root);

        while(!pending.empty()) {
            AVLNode * node = pending.front();
            pending.pop();

            if(node->left != nullptr)
                pending.push(node->left);
            if(node->right != nullptr)
                pending.push(node->right);

            toReturn.push_back(node);
        }

        return toReturn;
    }

    bool contains(uint32_t keyHashToSearch) const {
        return this->get(keyHashToSearch) != nullptr;
    }

    AVLNode * get(uint32_t keyHashToSearch) const {
        AVLNode * node = this->root;

        while (node != nullptr) {
            if (node->keyHash == keyHashToSearch)
                return node;
            else if (keyHashToSearch < node->keyHash)
                node = node->left;
            else
                node = node->right;
        }

        return nullptr;
    }

private:
    AVLNode * removeRecursive(AVLNode * last, uint32_t keyHashToRemove, uint64_t timestamp, uint16_t nodeId, bool& removed) {
        if(last == nullptr){
            return last;
        }else if(last->keyHash > keyHashToRemove) {
            last->left = this->removeRecursive(last->left, keyHashToRemove, timestamp, nodeId, removed);
        }else if (last->keyHash < keyHashToRemove) {
            last->right = this->removeRecursive(last->right, keyHashToRemove, timestamp, nodeId, removed);
        }else{ //Found it
            if(last->timestamp.compare(timestamp, nodeId)){ //Reject. Node has been updated by more updated node
                return last;
            }

            removed = true;
            bool rootRemoved = this->root == last;

            if(last->left && last->right) {
                AVLNode * temp = this->mostLeftChild(last->right);

                last->keyHash = temp->keyHash;
                last->value = temp->value;

                if(rootRemoved) this->root = last;

                bool ignore = false;
                last->right = removeRecursive(last->right, last->keyHash, last->timestamp.counter, last->timestamp.nodeId, ignore);
            }else{
                AVLNode * temp = last;
                if(last->left == nullptr)
                    last = last->right;
                else if(last->right == nullptr)
                    last = last->left;

                if(rootRemoved) this->root = last;

                temp->key.decreaseRefCount();
                temp->value.decreaseRefCount();
                delete temp;
            }
        }

        if(last != nullptr)
            last = this->rebalance(last);

        return last;
    }

    AVLNode * mostLeftChild(AVLNode * node) const {
        while (node->left != nullptr)
            node = node->left;

        return node;
    }

    AVLNode * insertRecursive(AVLNode * last, AVLNode * toInsert) {
        if(last == nullptr)
            return toInsert;

        if(last->keyHash > toInsert->keyHash) {
            AVLNode * inserted = insertRecursive(last->left, toInsert);
            if(inserted != nullptr) last->left = inserted;

        }else if(last->keyHash < toInsert->keyHash) {
            AVLNode * inserted = insertRecursive(last->right, toInsert);
            if(inserted != nullptr) last->right = inserted;

        }else{
            if(last->timestamp > toInsert->timestamp) //Reject. Node has been updated by more updated node
                return nullptr;

            last->keyHash = toInsert->keyHash;
            last->value = toInsert->value;
            last->key = toInsert->key;
            last->timestamp = toInsert->timestamp;
        }

        return this->rebalance(last);

//        return last->keyHash != toInsert->keyHash ?
//                this->rebalance(last) :
//                nullptr;
    }

    AVLNode * rebalance(AVLNode * node) {
        if(node == nullptr)
            return node;

        this->updateHeight(node);

        int16_t heightFactor = this->getHeightFactor(node);

        if(heightFactor < -1){ //Left heavy
            if(this->getHeightFactor(node->left) > 0)
                node->left = this->rotateLeft(node->left);
            node = this->rotateRight(node);
        }

        if(heightFactor > 1){ //Right heavy
            if(this->getHeightFactor(node->right) < 0)
                node->right = this->rotateRight(node->right);
            node = this->rotateLeft(node);
        }

        return node;
    }

    AVLNode * rotateRight(AVLNode * node) {
        AVLNode * leftChild = node->left;

        this->updateRootReferenceIfNeccesary(node, leftChild);

        node->left = leftChild->right;
        leftChild->right = node;

        this->updateHeight(node);
        this->updateHeight(leftChild);

        return leftChild;
    }


    AVLNode * rotateLeft(AVLNode * node) {
        AVLNode * rightChild = node->right;

        node->right = rightChild->left;
        rightChild->left = node;

        this->updateRootReferenceIfNeccesary(node, rightChild);

        this->updateHeight(node);
        this->updateHeight(rightChild);

        return rightChild;
    }

    void updateRootReferenceIfNeccesary(AVLNode * oldReference, AVLNode * newReference) {
        if(this->root == oldReference)
            this->root = newReference;
    }

    int16_t getHeightFactor(AVLNode * node) const {
        return node == nullptr ? 0 : this->getHeight(node->right) - this->getHeight(node->left);
    }

    void updateHeight(AVLNode * node) {
        int16_t rightHeight = this->getHeight(node->right);
        int16_t leftHeight = this->getHeight(node->left);

        node->height = std::max(leftHeight, rightHeight) + 1;
    }

    int16_t getHeight(AVLNode * node) const {
        return node == nullptr ? -1 : node->height;
    }
};