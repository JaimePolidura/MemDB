#include "utils/threads/pool/Worker.h"
#include "utils/threads/pool/DynamicThreadPool.h"

#include "shared.h"

class FixedThreadPool : public DynamicThreadPool {
public:
    FixedThreadPool(int numberThreads):
            DynamicThreadPool(std::numeric_limits<uint8_t>::max(), numberThreads, numberThreads ,std::numeric_limits<int>::max())
    {}

    FixedThreadPool() = default;
};