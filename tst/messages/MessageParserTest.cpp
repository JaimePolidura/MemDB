#include "gtest/gtest.h"
#include "messages/Message.h"
#include "messages/MessageParser.h"


TEST(MessageParser, WithArgs) {
    MessageParser messageParser{};

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

    std::shared_ptr<Message> parsedMessage = messageParser.parse(buffer);

    ASSERT_EQ(parsedMessage->operation->numberArgs, 2);

    OperatorArgument * firstArg = parsedMessage->operation->args;
    ASSERT_EQ(firstArg->lengthArg, 1);
    ASSERT_TRUE(arg1Expected.compare(std::string(firstArg->arg, firstArg->lengthArg)) == 0);

    OperatorArgument * secondArg = parsedMessage->operation->args + 1;
    ASSERT_EQ(secondArg->lengthArg, 2);
    ASSERT_TRUE(arg2Expected.compare(std::string(secondArg->arg, secondArg->lengthArg)) == 0);
}

TEST(MessageParser, EmptyArgs) {
    MessageParser messageParser{};

    std::vector<uint8_t> buffer = {
            0x0E, //00001110 -> Lengh: 3 Flag1: 1 Flag2: 0
            0x4C, // L
            0x4F, // O
            0x4C,  // L
            0x09 //000010 01 -> Operator number: 2, Flag1: 0, Flag2: 1
    };

    std::shared_ptr<Message> parsedMessage = messageParser.parse(buffer);

    ASSERT_TRUE(parsedMessage->authentication->flag1);
    ASSERT_FALSE(parsedMessage->authentication->flag2);
    ASSERT_EQ(parsedMessage->authentication->authKeyLength, 3);

    std::string authKey = "LOL";
    ASSERT_TRUE(authKey.compare(parsedMessage->authentication->authKey) == 0);

    ASSERT_FALSE(parsedMessage->operation->flag1);
    ASSERT_TRUE(parsedMessage->operation->flag2);
    ASSERT_TRUE(parsedMessage->operation->operatorNumber == 2);
}