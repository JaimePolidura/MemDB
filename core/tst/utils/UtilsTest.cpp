#include "gtest/gtest.h"
#include "utils/Utils.h"

TEST(Utils, parse) {
    auto toParse = std::vector<uint8_t>{0x00, 0x00, 0x01, 0xFF};
    auto parsed = Utils::parse<uint32_t>(toParse.data());

    ASSERT_EQ(511, parsed);
}

TEST(Utils, parseFromBuffer) {
    auto toParse = std::vector<uint8_t>{0x00, 0x00, 0x01, 0xFF};
    auto parsed = Utils::parseFromBuffer<uint32_t>(toParse);
    ASSERT_EQ(511, parsed);

    auto toParse2 = std::vector<uint8_t>{0x00, 0x00, 0x00, 0x01, 0xFF};
    auto parsed2 = Utils::parseFromBuffer<uint32_t>(toParse, 1);
    ASSERT_EQ(511, parsed);
}

TEST(Utils, appendToBufferValue) {
    std::vector<uint8_t> buffertoAppend{0x00};
    uint32_t valueToAppend = 12;
    Utils::appendToBuffer(valueToAppend, buffertoAppend);

    ASSERT_EQ(buffertoAppend.size(), 5);
    ASSERT_EQ(buffertoAppend[0], 0x00);
    ASSERT_EQ(buffertoAppend[1], 0x00);
    ASSERT_EQ(buffertoAppend[2], 0x00);
    ASSERT_EQ(buffertoAppend[3], 0x00);
    ASSERT_EQ(buffertoAppend[4], 12); //idk why 0x12 doest work
}

TEST(Utils, appendToBufferPointer) {
    std::vector<uint8_t> valuesToAppend{0x01, 0x02, 0x03, 0x04};
    std::vector<uint8_t> vectorToBeAppended{0x00};

    Utils::appendToBuffer(valuesToAppend.data(), 3, vectorToBeAppended);

    ASSERT_EQ(vectorToBeAppended.size(), 4);
    ASSERT_EQ(vectorToBeAppended[0], 0x00);
    ASSERT_EQ(vectorToBeAppended[1], 0x01);
    ASSERT_EQ(vectorToBeAppended[2], 0x02);
    ASSERT_EQ(vectorToBeAppended[3], 0x03);
}

TEST(Utils, copyFromBuffer) {
    auto copySource = std::vector<uint8_t>{0x00, 0x00, 0x01, 0xFF};
    auto copyResult = Utils::copyFromBuffer(copySource, 2, 3);

    ASSERT_EQ(* copyResult, 0x01);
    ASSERT_EQ(* (copyResult + 1), 0xFF);
}