#include "utils/threads/pool/Worker.h"
#include "utils/threads/pool/DynamicThreadPool.h"

#include <vector>

class FixedThreadPool : public DynamicThreadPool {
public:
    FixedThreadPool(int numberThreads):
            DynamicThreadPool(std::numeric_limits<uint8_t>::max(), numberThreads, numberThreads ,std::numeric_limits<int>::max(), "")
    {}
};