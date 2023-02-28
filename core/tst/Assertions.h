#pragma once

#include <vector>
#include <cstdint>
#include "gtest/gtest.h"

class Assertions {
public:
    template<class T>
    static void assertPointerValues(T * expected, T * actual, int until) {
        for (int i = 0; i < until; ++i) {
            ASSERT_EQ(* (expected + i), * (actual + i));
        }
    }

    static void assertFirstItemsVectorsEqual(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& toAssert) {
        for (int i = 0; i < expected.size(); ++i) {
            const uint8_t * expectedPtr = expected.data() + i;
            const uint8_t * toAssertPtr = toAssert.data() + i;

            ASSERT_EQ(* expectedPtr, * toAssertPtr);
        }
    }
};