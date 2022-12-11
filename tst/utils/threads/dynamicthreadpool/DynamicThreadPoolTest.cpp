#include "gtest/gtest.h"
#include "./utils/threads/dynamicthreadpool/DynamicThreadPool.h"

#include <functional>

void task(const DynamicThreadPool& pool);

TEST(DynamicThreadPool, ShouldEnqueue) {
    DynamicThreadPool threadPool{0.75f, 10, 2, 10};

    threadPool.submit([&threadPool]{task(threadPool);});
}

void task(const DynamicThreadPool& pool) {
    printf("HOla\n");
}
