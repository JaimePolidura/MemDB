#include "MemDbCreator.h"

int main() {
    std::shared_ptr<MemDb> memDb = MemDbCreator::create();
    memDb->run();

    return 0;
}