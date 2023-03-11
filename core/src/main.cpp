#include "MemDbCreator.h"

#include "shared.h"

int main() {
    etcd::Response res;

    printf("[SERVER] Initializing\n");
    std::shared_ptr<MemDb> memDb = MemDbCreator::create();
    memDb->run();

    return 0;
}
