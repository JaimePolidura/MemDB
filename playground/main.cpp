#include "utils/datastructures/tree/AVLTree.h"

int main() {
    AVLTree treee{};

    treee.add(2483, nullptr, 0);
    treee.add(2163, nullptr, 0);
    treee.add(2739, nullptr, 0);
    treee.contains(12);

    treee.remove(2483);
    treee.remove(2227);

    treee.contains(12);
}