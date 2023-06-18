#include "MemDbCreator.h"

#include "shared.h"

int main() {
    std::shared_ptr<MemDb> memDb = MemDbCreator::create();
    memDb->run();

    return 0;
}
