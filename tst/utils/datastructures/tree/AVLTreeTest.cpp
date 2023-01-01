#include "gtest/gtest.h"
#include "utils/datastructures/tree/AVLTree.h"

struct ToSave {
    int value;
};

TEST(AVLTree, ShouldRemoveNode) {
    AVLTree avlTree{};

    avlTree.add(1, (uint8_t *) new ToSave(), sizeof(ToSave));
    avlTree.add(5, (uint8_t *) new ToSave(), sizeof(ToSave));
    avlTree.add(4, (uint8_t *) new ToSave(), sizeof(ToSave));
    avlTree.add(2, (uint8_t *) new ToSave(), sizeof(ToSave));

    avlTree.remove(5);
    ASSERT_FALSE(avlTree.contains(5));
    ASSERT_TRUE(avlTree.contains(1));
    ASSERT_TRUE(avlTree.contains(4));
    ASSERT_TRUE(avlTree.contains(2));

    avlTree.remove(1);
    ASSERT_FALSE(avlTree.contains(1));
    ASSERT_TRUE(avlTree.contains(4));
    ASSERT_TRUE(avlTree.contains(2));

    avlTree.remove(4);
    ASSERT_FALSE(avlTree.contains(4));
    ASSERT_TRUE(avlTree.contains(2));

    avlTree.remove(2);
    ASSERT_FALSE(avlTree.contains(2));
}

TEST(AVLTree, ShouldRemoveRoot) {
    AVLTree avlTree{};
    ToSave * toValue_1 = new ToSave();

    avlTree.add(1, (uint8_t *) toValue_1, sizeof(ToSave));

    avlTree.remove(1);

    ASSERT_FALSE(avlTree.contains(1));
}

TEST(AVLTree, ShouldGet) {
    AVLTree avlTree{};

    ToSave * toValue_1 = new ToSave();
    toValue_1->value = 10;

    ASSERT_FALSE(avlTree.contains(1));

    avlTree.add(1, (uint8_t *) toValue_1, sizeof(ToSave));
    avlTree.add(3, (uint8_t *) toValue_1, sizeof(ToSave));
    avlTree.add(2, (uint8_t *) toValue_1, sizeof(ToSave));
    avlTree.add(5, (uint8_t *) toValue_1, sizeof(ToSave));

    ToSave * saved_2 = (ToSave *) avlTree.get(2)->value;
    ASSERT_TRUE(saved_2 != nullptr);
    ASSERT_EQ(saved_2->value, 10);

    ToSave * saved_5 = (ToSave *) avlTree.get(5)->value;
    ASSERT_TRUE(saved_5 != nullptr);
    ASSERT_EQ(saved_5->value, 10);

    ToSave * saved_1 = (ToSave *) avlTree.get(1)->value;
    ASSERT_TRUE(saved_1 != nullptr);
    ASSERT_EQ(saved_1->value, 10);
}

TEST(AVLTree, ShouldAdd) {
    AVLTree avlTree{};

    ToSave * toValue_1 = new ToSave();

    ASSERT_FALSE(avlTree.contains(1));

    avlTree.add(1, (uint8_t *) toValue_1, sizeof(ToSave));
    ASSERT_TRUE(avlTree.contains(1));

    avlTree.add(3, (uint8_t *) toValue_1, sizeof(ToSave));
    ASSERT_TRUE(avlTree.contains(3));

    avlTree.add(2, (uint8_t *) toValue_1, sizeof(ToSave));
    ASSERT_TRUE(avlTree.contains(2));

    avlTree.add(5, (uint8_t *) toValue_1, sizeof(ToSave));
    ASSERT_TRUE(avlTree.contains(5));
}