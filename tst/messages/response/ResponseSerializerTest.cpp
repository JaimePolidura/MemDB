#include "gtest/gtest.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/Response.h"
#include "messages/response/ErrorCode.h"

#include <string>
#include <memory>

TEST(ResponseSerializer, ShouldSerializeSuccessWithData) {
    ResponseSerializer responseSerializer{};

    std::string data = "hello";

    std::shared_ptr<Response> response = Response::success(data.length(), reinterpret_cast<uint8_t *>(data.data()));
    std::shared_ptr<std::vector<uint8_t>> serialized = responseSerializer.serialize(std::ref(* response));

    ASSERT_EQ(serialized->size(), 7);
    ASSERT_EQ(serialized->at(0), 0x01);
    ASSERT_EQ(serialized->at(1), 0x05);

    ASSERT_EQ(serialized->at(2), 0x68); //h
    ASSERT_EQ(serialized->at(3), 0x65); //e
    ASSERT_EQ(serialized->at(4), 0x6C); //l
    ASSERT_EQ(serialized->at(5), 0x6C); //l
    ASSERT_EQ(serialized->at(6), 0x6F); //o
}

TEST(ResponseSerializer, ShouldSerializeSuccessNoData) {
    ResponseSerializer responseSerializer{};

    std::shared_ptr<Response> response = Response::success();
    std::shared_ptr<std::vector<uint8_t>> serialized = responseSerializer.serialize(* response.get());

    ASSERT_EQ(serialized->size(), 2);
    ASSERT_EQ(serialized->at(0), 0x01);
    ASSERT_EQ(serialized->at(1), 0x00);
}

TEST(ResponseSerializer, ShouldSerializeErrorNoData) {
    ResponseSerializer responseSerializer{};

    std::shared_ptr<Response> response = Response::error(ErrorCode::UNKNOWN_KEY);
    std::shared_ptr<std::vector<uint8_t>> serialized = responseSerializer.serialize(* response.get());

    ASSERT_EQ(serialized->size(), 2);
    ASSERT_EQ(serialized->at(0), 0x02);
    ASSERT_EQ(serialized->at(1), 0x00);
}
