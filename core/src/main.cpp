#include "MemDbCreator.h"

#include "shared.h"

int main(int nArgs, char ** args) {
    std::shared_ptr<MemDb> memDb = MemDbCreator::create(nArgs, args);
    memDb->run();

    return 0;
}
