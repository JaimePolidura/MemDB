#pragma once

#include "DynamicThreadPool.h"

#include <limits>

class SingleThreadPool : public DynamicThreadPool {
public:
    SingleThreadPool(const std::string& name = ""):
        DynamicThreadPool(1.0f, 1, 1 ,std::numeric_limits<int>::max(), name)
    {}
};