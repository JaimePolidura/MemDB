#include "gtest/gtest.h"
#include <string>
#include "database/operators/GetOperator.h"

TEST(GetOperator, KeyNotFound) {
    std::shared_ptr<Map> db = std::make_shared<Map>();
    GetOperator getOperator{};

    uint8_t * key = new uint8_t[1]();
    key[0] = 0x41;
    OperatorArgument * argKey = new OperatorArgument(key, 1);
    OperationBody operation = {0, false, false, argKey, 1};

    std::shared_ptr<Response> response = getOperator.operate(operation, db);

    ASSERT_FALSE(response->isSuccessful);
    ASSERT_EQ(response->errorCode, UNKNOWN_KEY);
    ASSERT_EQ(response->lengthResponse, 0);
    ASSERT_EQ(response->response, nullptr);
}

TEST(GetOperator, KeyFound) {

}