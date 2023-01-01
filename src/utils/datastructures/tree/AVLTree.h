#pragma once

#include <cstdint>
#include <functional>

struct Node {
    Node * left;
    Node * right;
    uint8_t * value;
    uint32_t keyHash;
    uint8_t valueLength;
    uint8_t height;

    Node() = default;

    Node(uint8_t * value, uint32_t keyHash, uint8_t valueLength, uint8_t height):
            left(nullptr), right(nullptr), value(value), keyHash(keyHash), valueLength(valueLength), height(height) {
    }
};

class AVLTree {
public:
    Node * root;

public:
    void add(int keyHash, uint8_t * value, uint8_t valueLength) {
        Node * newNode = new Node(value, keyHash, valueLength, -1);

        if(this->root == nullptr)
            this->root = newNode;
        else
            this->insertRecursive(this->root, newNode);
    }

    void remove(uint32_t keyHash) {
        this->removeRecursive(this->root, keyHash);
    }

    bool contains(uint32_t keyHashToSearch) {
        return this->get(keyHashToSearch) != nullptr;
    }

    Node * get(int keyHashToSearch) {
        Node * node = this->root;

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
    Node * removeRecursive(Node * last, uint32_t keyHashToRemove) {
        if(last == nullptr){
            return last;
        }else if(last->keyHash > keyHashToRemove) {
            last->left = this->removeRecursive(last->left, keyHashToRemove);
        }else if (last->keyHash < keyHashToRemove) {
            last->right = this->removeRecursive(last->right, keyHashToRemove);
        }else{ //Found it
            Node * left = last->left;
            Node * right = last->right;
            uint32_t keyHash = last->keyHash;

            delete[] last->value;
            delete last;

            if(left == nullptr || right == nullptr) {
                last = (left == nullptr) ? right : left;
            }else{
                last = this->mostLeftChild(right);
                last->right = this->removeRecursive(right, keyHash);
            }
        }

        if(last != nullptr)
            this->rebalance(last);

        return last;
    }

    Node * mostLeftChild(Node * node) {
        while (node->left != nullptr) {
            node = node->left;
        }

        return node;
    }

    Node * insertRecursive(Node * last, Node * toInsert) {
        if(last == nullptr){
            return toInsert;
        }else if(last->keyHash > toInsert->keyHash) {
            last->left = insertRecursive(last->left, toInsert);
        }else if(last->keyHash < toInsert->keyHash) {
            last->right = insertRecursive(last->right, toInsert);
        }

        return this->rebalance(last);
    }

    Node * rebalance(Node * node) {
        this->updateHeight(node);

        int8_t heightFactor = this->getHeightFactor(node);

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

    Node * rotateRight(Node * node) {
        Node * leftChild = node->left;

        this->updateRootReferenceIfNeccesary(node, leftChild);

        node->left = leftChild->right;
        leftChild->right = node;

        this->updateHeight(node);
        this->updateHeight(leftChild);

        return leftChild;
    }


    Node * rotateLeft(Node * node) {
        Node * rightChild = node->right;

        node->right = rightChild->left;
        rightChild->left = node;

        this->updateRootReferenceIfNeccesary(node, rightChild);

        this->updateHeight(node);
        this->updateHeight(rightChild);

        return rightChild;
    }

    void updateRootReferenceIfNeccesary(Node * oldReference, Node * newReference) {
        if(this->root == oldReference)
            this->root = newReference;
    }

    int8_t getHeightFactor(Node * node) {
        return node == nullptr ? 0 : this->getHeight(node->right) - this->getHeight(node->left);
    }

    void updateHeight(Node * node) {
        uint8_t rightHeight = this->getHeight(node->right);
        uint8_t leftHeight = this->getHeight(node->left);

        node->height = std::max(leftHeight, rightHeight) + 1;
    }

    uint8_t getHeight(Node * node) {
        return node == nullptr ? -1 : node->height;
    }
};