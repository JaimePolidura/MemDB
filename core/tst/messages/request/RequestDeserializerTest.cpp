#include "gtest/gtest.h"
#include "messages/request/Request.h"
#include "messages/request/RequestDeserializer.h"
#include <string>

TEST(RequesteRequestDeserializer, WithArgsAndNodeId) {
    RequestDeserializer requestDeserializer{};
    std::string expectedArg1 = "A";
    std::string expectedArg2 = "BC";
    std::string authKeyExpected = "LOL";

    std::vector<uint8_t> buffer = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, //Req number -> 511
            0x0C, 0x4C, 0x4F, 0x4C, //Auth key -> LOL
            0x09, //Opdesc -> op num: 0x02, flag1: 0, flag2: 1
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, //Timestamp -> 255
            0x02, 0xFF, //NodeNumber -> 767
            0x01, 0x41, //A
            0x02, 0x42, 0x43 //B
    };

    Request deserializedRequest = requestDeserializer.deserialize(buffer, true);

    ASSERT_EQ(deserializedRequest.requestNumber, 511);

    ASSERT_FALSE(deserializedRequest.authentication.flag1);
    ASSERT_FALSE(deserializedRequest.authentication.flag2);
    ASSERT_TRUE(authKeyExpected.compare(deserializedRequest.authentication.authKey) == 0);

    ASSERT_EQ(deserializedRequest.operation.operatorNumber, 0x02);
    ASSERT_FALSE(deserializedRequest.operation.flag1);
    ASSERT_TRUE(deserializedRequest.operation.flag2);
    ASSERT_EQ(deserializedRequest.operation.timestamp, 255);
    ASSERT_EQ(deserializedRequest.operation.nodeId, 767);
    ASSERT_EQ(deserializedRequest.operation.args->size(), 2);

    SimpleString firstArg = deserializedRequest.operation.args->at(0);
    ASSERT_EQ(firstArg.size, 1);
    ASSERT_TRUE(expectedArg1.compare(std::string((char *) firstArg.data(), firstArg.size)) == 0);

    SimpleString secondArg = deserializedRequest.operation.args->at(1);
    ASSERT_EQ(secondArg.size, 2);
    ASSERT_TRUE(expectedArg2.compare(std::string((char *) secondArg.data(), secondArg.size)) == 0);
}

TEST(RequesteRequestDeserializer, WithArgsNoNodeId) {
    RequestDeserializer requestDeserializer{};
    std::string expectedArg1 = "A";
    std::string expectedArg2 = "BC";
    std::string authKeyExpected = "LOL";

    std::vector<uint8_t> buffer = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, //Req number -> 511
            0x0C, 0x4C, 0x4F, 0x4C, //Auth key -> LOL
            0x09, //Opdesc -> op num: 0x02, flag1: 0, flag2: 1
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, //Timestamp -> 255
            0x01, 0x41, //A
            0x02, 0x42, 0x43 //B
    };

    Request deserializedRequest = requestDeserializer.deserialize(buffer);

    ASSERT_EQ(deserializedRequest.requestNumber, 511);

    ASSERT_FALSE(deserializedRequest.authentication.flag1);
    ASSERT_FALSE(deserializedRequest.authentication.flag2);
    ASSERT_TRUE(authKeyExpected.compare(deserializedRequest.authentication.authKey) == 0);

    ASSERT_EQ(deserializedRequest.operation.operatorNumber, 0x02);
    ASSERT_FALSE(deserializedRequest.operation.flag1);
    ASSERT_TRUE(deserializedRequest.operation.flag2);
    ASSERT_EQ(deserializedRequest.operation.timestamp, 255);
    ASSERT_EQ(deserializedRequest.operation.args->size(), 2);

    SimpleString firstArg = deserializedRequest.operation.args->at(0);
    ASSERT_EQ(firstArg.size, 1);
    ASSERT_TRUE(expectedArg1.compare(std::string((char *) firstArg.data(), firstArg.size)) == 0);

    SimpleString secondArg = deserializedRequest.operation.args->at(1);
    ASSERT_EQ(secondArg.size, 2);
    ASSERT_TRUE(expectedArg2.compare(std::string((char *) secondArg.data(), secondArg.size)) == 0);
}

TEST(RequesteRequestDeserializer, EmptyArgsNodeId) {
    RequestDeserializer requestDeserializer{};
    std::string authKeyExpected = "LOL";

    std::vector<uint8_t> buffer = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Req number -> 1
            0x0C, 0x4C, 0x4F, 0x4C, //Auth key -> LOL
            0x09, //Operator number: 2, Flag1: 0, Flag2: 1
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, //Timestamp -> 255
            0x02, 0xFF //NodeNumber -> 767
    };

    Request deserializedRequest = requestDeserializer.deserialize(buffer, true);

    ASSERT_EQ(deserializedRequest.requestNumber, 1);

    ASSERT_FALSE(deserializedRequest.authentication.flag1);
    ASSERT_FALSE(deserializedRequest.authentication.flag2);
    ASSERT_TRUE(authKeyExpected.compare(deserializedRequest.authentication.authKey) == 0);

    ASSERT_EQ(deserializedRequest.operation.operatorNumber, 0x02);
    ASSERT_FALSE(deserializedRequest.operation.flag1);
    ASSERT_TRUE(deserializedRequest.operation.flag2);
    ASSERT_EQ(deserializedRequest.operation.timestamp, 255);
    ASSERT_EQ(deserializedRequest.operation.nodeId, 767);
    ASSERT_EQ(deserializedRequest.operation.args->size(), 0);
}