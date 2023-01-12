#include "utils/datastructures/tree/AVLTree.h"

int main() {
    AVLTree tree{};

    tree.remove(2252);
    tree.remove(2124);
    tree.add(2188, nullptr, 0);
    tree.add(2828, nullptr, 0);
    tree.remove(2380);
    tree.add(2828, nullptr, 0);
    tree.add(2316, nullptr, 0);
    tree.add(2252, nullptr, 0);
    tree.remove(2188);
    tree.add(2828, nullptr, 0);
    tree.remove(2188);
    tree.remove(2124);
    tree.add(2700, nullptr, 0);
    tree.add(2444, nullptr, 0);
    tree.remove(2124);
    tree.remove(2252);
    tree.add(2188, nullptr, 0);
    tree.add(2444, nullptr, 0);
    tree.add(2188, nullptr, 0);
    tree.add(2316, nullptr, 0);
    tree.remove(2252);
    tree.remove(2124);
    tree.add(2380, nullptr, 0);
    tree.add(2252, nullptr, 0);
    tree.add(2124, nullptr, 0);
    tree.add(2828, nullptr, 0);
    tree.remove(2316);
    tree.add(2828, nullptr, 0);
    tree.contains(1);
}