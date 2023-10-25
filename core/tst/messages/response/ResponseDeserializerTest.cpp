#include "gtest/gtest.h"

#include "messages/response/ResponseDeserializer.h"
#include "../../Assertions.h"

#include <string>

TEST(ResponseDeserializer, ResponseWithArgSuccess) {
    ResponseDeserializer responseDeserializer{};
    auto responseDeserialized = responseDeserializer.deserialize({
        0x00, 0x00, 0x00, 0x02, //Request number
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Timestamp
        0x01, //Success
        0x00, 0x00, 0x00, 0x05, //Length response
        0x68, 0x65, 0x6C, 0x6C, 0x6F //hello
    });

    ASSERT_EQ(responseDeserialized.requestNumber, 2);
    ASSERT_EQ(responseDeserialized.timestamp, 1);
    ASSERT_TRUE(responseDeserialized.isSuccessful);

    ASSERT_EQ(responseDeserialized.responseValue.size, 5);

    std::vector<uint8_t> expectedArgValue = {0x68, 0x65, 0x6C, 0x6C, 0x6F};
    Assertions::assertPointerValues(expectedArgValue.data(), responseDeserialized.responseValue.data(),
                                    responseDeserialized.responseValue.size);
}

TEST(ResponseDeserializer, ResponseWithoutArgsError) {
    ResponseDeserializer responseDeserializer{};
    auto responseDeserialized = responseDeserializer.deserialize({
        0x00, 0x00, 0x00, 0x02, //Request number
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Timestamp
        0x02, //_error errorcode 1
        0x00, 0x00, 0x00, 0x00, //Length response
    });

    ASSERT_EQ(responseDeserialized.requestNumber, 2);
    ASSERT_EQ(responseDeserialized.timestamp, 1);
    ASSERT_FALSE(responseDeserialized.isSuccessful);
    ASSERT_EQ(responseDeserialized.errorCode, 1);

    ASSERT_EQ(responseDeserialized.responseValue.size, 0);
}