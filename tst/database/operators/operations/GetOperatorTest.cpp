#include "gtest/gtest.h"
#include <string>

#include "database/operators/operations/GetOperator.h"
#include "messages/response/ErrorCode.h"

std::shared_ptr<OperationBody> createOperation(uint8_t keyValue);

TEST(GetOperator, CorrectConfig) {
    GetOperator getOperator{};

    ASSERT_EQ(getOperator.type(), READ);
    ASSERT_EQ(getOperator.operatorNumber(), 0x02);
}

TEST(GetOperator, KeyNotFound) {
    std::shared_ptr<Map> db = std::make_shared<Map>();
    GetOperator getOperator{};

    auto operation = createOperation(0x41); //A

    std::shared_ptr<Response> response = getOperator.operate(operation, db);

    ASSERT_FALSE(response->isSuccessful);
    ASSERT_EQ(response->errorCode, 0x01);
    ASSERT_EQ(response->lengthResponse, 0);
    ASSERT_EQ(response->response, nullptr);
}

TEST(GetOperator, KeyFound) {
    std::shared_ptr<Map> db = std::make_shared<Map>();
    uint8_t * value = new uint8_t[3]{0x4C, 0x4F ,0x4C}; //LOL
    db->put("A", value, 3);
    GetOperator getOperator{};
    auto operation = createOperation(0x41); //A

    std::shared_ptr<Response> response = getOperator.operate(operation, db);

    ASSERT_TRUE(response->isSuccessful);
    ASSERT_EQ(response->errorCode, 0);
    ASSERT_EQ(response->lengthResponse, 3);
    ASSERT_EQ(* response->response, 0x4C);
    ASSERT_EQ(* (response->response + 1), 0x4F);
    ASSERT_EQ(* (response->response + 2), 0x4C);
}

std::shared_ptr<OperationBody> createOperation(uint8_t keyValue) {
    uint8_t * key = new uint8_t[1]();
    key[0] = 0x41;
    std::vector<OperatorArgument> args;
    args.emplace_back(key, 1);

    return std::make_shared<OperationBody>(0, false, false, args, 1);
}