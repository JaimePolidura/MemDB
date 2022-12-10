#include "gtest/gtest.h"
#include "utils/threads/Exchanger.h"
#include <string>

struct ExchangeItemTest {
public:
    int value;

    ExchangeItemTest(int valueCons): value(valueCons) {}
};

TEST(Exchanger, ShouldEnqueueAndDequeue) {
    Exchanger<ExchangeItemTest> exchanger;

    std::shared_ptr<ExchangeItemTest> itemTest = std::make_shared<ExchangeItemTest>(1);

    std::thread tEnqueuer([&exchanger, &itemTest]{
        exchanger.enqueue(itemTest);
        ASSERT_TRUE(exchanger.isEmpty());
    });

    std::thread tDequeuer([&exchanger]{
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::shared_ptr<ExchangeItemTest> dequeuedItem = exchanger.dequeue();

        ASSERT_TRUE(dequeuedItem->value == 1);
        ASSERT_TRUE(exchanger.isEmpty());
    });

    tEnqueuer.join();
    tDequeuer.join();

    ASSERT_TRUE(exchanger.isEmpty());
}