#include "gtest/gtest.h"
#include "utils/datastructures/tree/AVLTree.h"

struct ToSave {
    int value;
};

TEST(AVLTree, ShouldAddNewKeys) {
    AVLTree avlTree{};

    bool addedA = avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 1, 1);
    ASSERT_TRUE(addedA);
    ASSERT_TRUE(avlTree.contains(1));
    auto addedNodeA = avlTree.get(1);
    ASSERT_TRUE(addedNodeA->keyHash == 1 && * addedNodeA->key.value == 'A');

    bool addedB = avlTree.add(SimpleString::fromChar('B'), 2, SimpleString::fromChar('B'), 1, 1);
    ASSERT_TRUE(addedB);
    ASSERT_TRUE(avlTree.contains(2));
    auto addedNodeB = avlTree.get(2);
    ASSERT_TRUE(addedNodeB->keyHash == 2 && * addedNodeB->key.value == 'B');
}


TEST(AVLTree, ShouldReplaceKeys) {
    AVLTree avlTree{};

    avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 1, 1);
    bool replaced = avlTree.add(SimpleString::fromChar('B'), 1, SimpleString::fromChar('B'), 2, 1);

    ASSERT_TRUE(replaced);
    ASSERT_TRUE(avlTree.contains(1));

    auto replacedNode = avlTree.get(2);
    ASSERT_TRUE(replacedNode->keyHash == 1 &&
        *replacedNode->key.value == 'B' &&
        replacedNode->timestamp.nodeId == 1 &&
        replacedNode->timestamp.counter == 2);
}

TEST(AVLTree, ShouldntReplaceKeysDifferentCounter) {
    AVLTree avlTree{};

    avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 2, 1);
    auto addedNode = avlTree.get(1);

    bool replaced = avlTree.add(SimpleString::fromChar('B'), 1, SimpleString::fromChar('B'), 1, 1);
    ASSERT_FALSE(replaced);
    ASSERT_TRUE(addedNode->keyHash == 1 &&
                *addedNode->key.value == 'A' &&
                addedNode->timestamp.nodeId == 1 &&
                addedNode->timestamp.counter == 2);
}

TEST(AVLTree, ShouldntReplaceKeysDifferentNodeId) {
    AVLTree avlTree{};

    avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 1, 2);
    auto addedNode = avlTree.get(1);

    bool replaced = avlTree.add(SimpleString::fromChar('B'), 1, SimpleString::fromChar('B'), 1, 1);
    ASSERT_FALSE(replaced);
    ASSERT_TRUE(addedNode->keyHash == 1 &&
                *addedNode->key.value == 'A' &&
                addedNode->timestamp.nodeId == 1 &&
                addedNode->timestamp.counter == 2);
}

TEST(AVLTree, ShouldGetAll) {
    AVLTree avlTree{};
    avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 1, 2);
    avlTree.add(SimpleString::fromChar('A'), 2, SimpleString::fromChar('A'), 1, 2);
    avlTree.add(SimpleString::fromChar('A'), 3, SimpleString::fromChar('A'), 1, 2);
    avlTree.add(SimpleString::fromChar('A'), 4, SimpleString::fromChar('A'), 1, 2);
    avlTree.add(SimpleString::fromChar('A'), 5, SimpleString::fromChar('A'), 1, 2);

    auto allNodes = avlTree.all();

    ASSERT_TRUE(allNodes.size() == 5);
}

TEST(AVLTree, ShouldntGetAllEmpty) {
    AVLTree avlTree{};
    auto allNodes = avlTree.all();
    ASSERT_TRUE(allNodes.empty());
}

/**
 *           4                   4
 *         /   \               /   \
 *        2     6             2     7
 *       / \   / \     -->   / \   / \
 *      1   3 5   7         1   3 5   6
 *                 \
 *                  8
 */
TEST(AVLTree, RemoveBiggerTree) {
    AVLTree avlTree{};

    avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 2, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 3, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 4, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 5, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 6, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 7, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 8, SimpleString::fromChar('A'), 1, 1);

    avlTree.remove(6, 2, 1);

    auto rootNode = avlTree.get(4);
    auto leftToRootNode = rootNode->left;
    auto rightToRootNode = rootNode->right;

    ASSERT_TRUE(rootNode->keyHash == 4);
    ASSERT_TRUE(leftToRootNode->keyHash == 2);
    ASSERT_TRUE(rightToRootNode->keyHash == 7);
    ASSERT_TRUE(leftToRootNode->left->keyHash == 1);
    ASSERT_TRUE(leftToRootNode->right->keyHash == 3);
    ASSERT_TRUE(rightToRootNode->left->keyHash == 5);
    ASSERT_TRUE(rightToRootNode->right->keyHash == 6);
}

/**
 *    2            3
 *   / \    ->    /
 *  1   3        1
 */
TEST(AVLTree, RemoveRoot) {
    AVLTree avlTree{};
    avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 2, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 3, SimpleString::fromChar('A'), 1, 1);

    avlTree.remove(2, 2, 1); //Remove
    ASSERT_FALSE(avlTree.contains(2));
    ASSERT_TRUE(avlTree.contains(1));
    ASSERT_TRUE(avlTree.contains(3));

    avlTree.remove(3, 0, 0); //Shouldn't remove
    ASSERT_TRUE(avlTree.contains(1));
    ASSERT_TRUE(avlTree.contains(3));

    avlTree.remove(3, 2, 0); //Remove
    ASSERT_FALSE(avlTree.contains(3));
    ASSERT_TRUE(avlTree.contains(1));
}

/**
 *    2         2           2
 *   / \    ->   \     ->
 *  1   3         3
 */
TEST(AVLTree, RemoveLeaf) {
    AVLTree avlTree{};
    avlTree.add(SimpleString::fromChar('A'), 1, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 2, SimpleString::fromChar('A'), 1, 1);
    avlTree.add(SimpleString::fromChar('A'), 3, SimpleString::fromChar('A'), 1, 1);

    avlTree.remove(1, 2, 1); //Remove
    ASSERT_FALSE(avlTree.contains(1));
    ASSERT_TRUE(avlTree.contains(2));
    ASSERT_TRUE(avlTree.contains(3));

    avlTree.remove(3, 0, 0); //Shouldn't remove
    ASSERT_TRUE(avlTree.contains(2));
    ASSERT_TRUE(avlTree.contains(3));

    avlTree.remove(3, 2, 1); //Remove
    ASSERT_FALSE(avlTree.contains(3));
    ASSERT_TRUE(avlTree.contains(2));
}