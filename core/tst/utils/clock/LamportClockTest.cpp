#include "gtest/gtest.h"
#include "utils/clock/LamportClock.h"

TEST(LamportClock, Comparation) {
    LamportClock a{2, 2}; //nodeId - count
    LamportClock b{2, 1};

    ASSERT_TRUE(a > b);
    ASSERT_FALSE(b > a);

    LamportClock c{1, 1}; //nodeId - count
    LamportClock d{2, 1};

    ASSERT_TRUE(d > c);
    ASSERT_FALSE(c > d);
}

TEST(LamportClock, Tick) {
    LamportClock clock{2};

    ASSERT_EQ(clock.tick(2), 3);
    ASSERT_EQ(clock.tick(1), 4);
    ASSERT_EQ(clock.tick(5), 6);
}

TEST(LamportClock, Initialization) {
    LamportClock clock{2};

    ASSERT_EQ(clock.nodeId, 2);
    ASSERT_EQ(clock.counter, 0);
}
