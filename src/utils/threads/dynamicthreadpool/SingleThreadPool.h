#pragma once

#include "DynamicThreadPool.h"

#include <limits>

class SingleThreadPool : public DynamicThreadPool {
public:
    SingleThreadPool(): DynamicThreadPool(1.0f, 1, 1 ,std::numeric_limits<int>::max()) {}
};