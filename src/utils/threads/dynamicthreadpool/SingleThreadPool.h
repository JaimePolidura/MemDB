#pragma once

#include "DynamicThreadPool.h"

#include <limits>

class SingleThreadPool : public DynamicThreadPool {
public:
    SingleThreadPool(const std::string& name = ""):
        DynamicThreadPool(std::numeric_limits<uint8_t>::max(), 1, 1 ,std::numeric_limits<int>::max(), name)
    {}
};