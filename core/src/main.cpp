#include "MemDbCreator.h"
#include <etcd/Client.hpp>

int main() {
    printf("[SERVER] Initializing\n");

    std::shared_ptr<MemDb> memDb = MemDbCreator::create();
    memDb->run();

    return 0;
}
