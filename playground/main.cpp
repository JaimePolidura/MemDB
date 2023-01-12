#include "utils/datastructures/tree/AVLTree.h"

int main() {
    AVLTree treee{};

    treee.add(2458, nullptr, 0);
    treee.remove(2330);
    treee.add(2714, nullptr, 0);
    treee.add(2586, nullptr, 0);
    treee.add(2394, nullptr, 0);
    treee.add(2778, nullptr, 0);
    treee.add(2842, nullptr, 0);
    treee.add(2714, nullptr, 0);
    treee.add(2714, nullptr, 0);
    treee.remove(2586);
    treee.remove(2458);

    treee.contains(12);
}