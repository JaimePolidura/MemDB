#include "gtest/gtest.h"
#include "utils/strings/StringUtils.h"

TEST(StringUtils, toString) {
    auto parsedToString = StringUtils::toString(12);

    ASSERT_TRUE(parsedToString.compare("12") == 0);
}

TEST(StringUtils, Split) {
    std::vector<std::string> splited = StringUtils::split("A B C D", ' ');
    ASSERT_EQ(splited.size(), 4);
    ASSERT_EQ(splited.at(0), "A");
    ASSERT_EQ(splited.at(1), "B");
    ASSERT_EQ(splited.at(2), "C");
    ASSERT_EQ(splited.at(3), "D");
}