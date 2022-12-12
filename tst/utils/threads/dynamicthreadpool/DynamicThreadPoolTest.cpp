#include "gtest/gtest.h"
#include "./utils/threads/dynamicthreadpool/DynamicThreadPool.h"

#include <functional>

void task(const DynamicThreadPool& pool, int& taskRunned);

TEST(DynamicThreadPool, ShouldScaleUp) {
    DynamicThreadPool threadPool{0.5f, 2, 1, 1};

    threadPool.submit([]{std::this_thread::sleep_for(std::chrono::seconds(1));}); //Asi el worker esta activo

    if(threadPool.getNumberWorkers() != 2)
        printf("ShouldScaleUp [NOT PASSED]\n");
    if(threadPool.getNumberWorkers() == 2)
        printf("ShouldScaleUp [PASSED]\n");
}

TEST(DynamicThreadPool, ShouldEnqueue) {
    int taskRunned = 0;
    DynamicThreadPool threadPool{0.75f, 10, 2, 10};

    threadPool.submit([&threadPool, &taskRunned]{task(threadPool, taskRunned);});

    std::this_thread::sleep_for(std::chrono::seconds(1));

    threadPool.stop();

    if(taskRunned != 1)
        printf("ShouldEnqueue [NOT PASSED]\n");
    if(taskRunned == 1)
        printf("ShouldEnqueue [PASSED]\n");
}

void task(const DynamicThreadPool& pool, int& taskRunned) {
    taskRunned++;
}
