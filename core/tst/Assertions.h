#pragma once

#include <vector>
#include <cstdint>
#include <gtest/gtest.h>
#include <functional>

#include "messages/request/Request.h"

class Assertions {
public:
    template<typename T>
    static void assertAnyMatch(const std::vector<T>& toTest, std::function<bool(const T&)> predicate) {
        bool predicateMatched = false;
        for(auto i = toTest.begin(); i < toTest.end(); i++){
            predicateMatched = predicate(* i);

            if(predicateMatched)
                break;
        }

        ASSERT_TRUE(predicateMatched);
    }

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