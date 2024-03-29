#include "gtest/gtest.h"

#include <memory>
#include <string>

#include "messages/response/ErrorCode.h"
#include "messages/request/Request.h"
#include "operators/operations/user/GetOperator.h"

OperationBody createOperationGet(uint8_t keyValue, uint64_t timestamp, uint16_t nodeId);

TEST(GetOperator, CorrectConfig) {
    GetOperator getOperator{};

    ASSERT_EQ(getOperator.type(), DB_STORE_READ);
    ASSERT_EQ(getOperator.operatorNumber(), GetOperator::OPERATOR_NUMBER);
}

TEST(GetOperator, KeyNotFound) {
    memDbDataStore_t db = std::make_shared<Map<memDbDataLength_t>>(64);
    GetOperator getOperator{};

    //A
    Response response = getOperator.operate(
            createOperationGet(0x41, 1, 1),
            OperationOptions{.checkTimestamps=false},
            OperatorDependencies{.dbStore = db});

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, 0x01);
    ASSERT_EQ(response.responseValue.size, 0);
    ASSERT_EQ(response.responseValue.data(), nullptr);
}

TEST(GetOperator, KeyFound) {
    memDbDataStore_t db = std::make_shared<Map<memDbDataLength_t>>(64);
    SimpleString key = SimpleString<memDbDataLength_t>::fromChar('A');
    SimpleString value = SimpleString<memDbDataLength_t>::fromArray({0x4C, 0x4F, 0x4C});

    db->put(key, value, 1, 1, false);

    GetOperator getOperator{};

    //A
    Response response = getOperator.operate(
            createOperationGet(0x41, 1, 1),
            OperationOptions{.checkTimestamps=false},
            OperatorDependencies{.dbStore = db});

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, 0);
    ASSERT_EQ(response.responseValue.size, 3);
    ASSERT_EQ(* response.responseValue.data(), 0x4C);
    ASSERT_EQ(* (response.responseValue + 1), 0x4F);
    ASSERT_EQ(* (response.responseValue + 2), 0x4C);
}

OperationBody createOperationGet(uint8_t keyValue, uint64_t timestamp, uint16_t nodeId) {
    uint8_t * keyRawPtr = new uint8_t();
    * keyRawPtr = keyValue;
    std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>> args = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
    args->emplace_back(keyRawPtr, 1);

    return OperationBody(GetOperator::OPERATOR_NUMBER, false, false, timestamp, nodeId, args);
}