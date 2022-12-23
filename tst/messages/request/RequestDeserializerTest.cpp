#include "gtest/gtest.h"
#include "messages/request/Request.h"
#include "messages/request/RequestDeserializer.h"
#include <string>

TEST(RequesteRequestDeserializer, WithArgs) {
    RequestDeserializer requestDeserializer{};

    std::vector<uint8_t> buffer = {
            0x0C, //000011 00 -> Lengh: 3 Flag1: 0 Flag2: 0
            0x4C, //L
            0x4F, //O
            0x4C, //L
            0x09, //000010 01 -> Operator number: 2, Flag1: 0, Flag2: 1
            0x01, //Arg nº1 lengh 1
            0x41, //A
            0x02, //Arg nº2 length: 2
            0x42, //B
            0x43  //C
    };

    std::string arg1Expected = "A";
    std::string arg2Expected = "BC";

    std::shared_ptr<Request> deserializedRequest = requestDeserializer.deserialize(buffer);

    ASSERT_EQ(deserializedRequest->operation->numberArgs, 2);

    OperatorArgument firstArg = deserializedRequest->operation->args.at(0);
    ASSERT_EQ(firstArg.lengthArg, 1);
    ASSERT_TRUE(arg1Expected.compare(std::string((char *) firstArg.arg, firstArg.lengthArg)) == 0);

    OperatorArgument secondArg = deserializedRequest->operation->args.at(1);
    ASSERT_EQ(secondArg.lengthArg, 2);
    ASSERT_TRUE(arg2Expected.compare(std::string((char *) secondArg.arg, secondArg.lengthArg)) == 0);
}

TEST(RequesteRequestDeserializer, EmptyArgs) {
    RequestDeserializer requestDeserializer{};

    std::vector<uint8_t> buffer = {
            0x0E, //00001110 -> Lengh: 3 Flag1: 1 Flag2: 0
            0x4C, // L
            0x4F, // O
            0x4C,  // L
            0x09 //000010 01 -> Operator number: 2, Flag1: 0, Flag2: 1
    };

    std::shared_ptr<Request> deserializerRequest = requestDeserializer.deserialize(buffer);

    ASSERT_TRUE(deserializerRequest->authentication->flag1);
    ASSERT_FALSE(deserializerRequest->authentication->flag2);
    ASSERT_EQ(deserializerRequest->authentication->authKey.size(), 3);

    std::string authKey = "LOL";
    ASSERT_TRUE(authKey.compare(deserializerRequest->authentication->authKey) == 0);

    ASSERT_FALSE(deserializerRequest->operation->flag1);
    ASSERT_TRUE(deserializerRequest->operation->flag2);
    ASSERT_TRUE(deserializerRequest->operation->operatorNumber == 2);
}