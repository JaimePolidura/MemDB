#include "gtest/gtest.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/Response.h"
#include "messages/response/ErrorCode.h"

#include <string>

void assertFirstItemsVectorsEqual(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& toAssert);

TEST(ResponseSerializer, ShouldSerializeSuccessWithData) {
    ResponseSerializer responseSerializer{};

    Response response = Response::success(SimpleString::fromArray({0x68, 0x65, 0x6C, 0x6C, 0x6F}));
    response.requestNumber = 0x02;
    response.timestamp = 0x01;

    std::vector<uint8_t> serialized = responseSerializer.serialize(response);
    std::vector<uint8_t> expected = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, //Request number
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Timestamp
            0x01, //Success
            0x05, //Length response
            0x68, 0x65, 0x6C, 0x6C, 0x6F //hello
    };

    assertFirstItemsVectorsEqual(expected, serialized);
}

TEST(ResponseSerializer, ShouldSerializeSuccessNoData) {
    ResponseSerializer responseSerializer{};

    Response response = Response::success();
    response.timestamp = 0x01;
    response.requestNumber = 0x02;

    std::vector<uint8_t> serialized = responseSerializer.serialize(response);
    std::vector<uint8_t> expected = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, //Request number
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Timestamp
            0x01, //Success
            0x00  //Length response
    };

    assertFirstItemsVectorsEqual(expected, serialized);
}

TEST(ResponseSerializer, ShouldSerializeErrorNoData) {
    ResponseSerializer responseSerializer{};

    Response response = Response::error(ErrorCode::UNKNOWN_KEY);
    response.timestamp = 0x01;
    response.requestNumber = 0x02;

    std::vector<uint8_t> serialized = responseSerializer.serialize(response);
    std::vector<uint8_t> expected = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, //Request number
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Timestamp
            0x02, //Error
            0x00  //Length response
    };

    assertFirstItemsVectorsEqual(expected, serialized);
}


void assertFirstItemsVectorsEqual(const std::vector<uint8_t>& expected, const std::vector<uint8_t>& toAssert) {
    for (int i = 0; i < expected.size(); ++i) {
        const uint8_t * expectedPtr = expected.data() + i;
        const uint8_t * toAssertPtr = toAssert.data() + i;

        ASSERT_EQ(* expectedPtr, * toAssertPtr);
    }
}