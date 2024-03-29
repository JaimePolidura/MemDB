#include "gtest/gtest.h"
#include "messages/request/Request.h"
#include "operators/operations/user/SetOperator.h"
#include "messages/response/ErrorCode.h"
#include "auth/AuthenticationType.h"

#include <string>
#include <memory>

OperationBody createOperationSet(uint8_t keyValue, uint8_t valueValue, uint64_t timestamp, uint16_t nodeId);

TEST(SetOperator, CorrectConfig) {
    SetOperator setOperator{};

    ASSERT_EQ(setOperator.type(), DB_STORE_WRITE);
    ASSERT_EQ(setOperator.operatorNumber(), SetOperator::OPERATOR_NUMBER);
}

TEST(SetOperator, ShouldtReplaceEvenNewerKeyTimestamp) {
    memDbDataStore_t db = std::make_shared<Map<memDbDataLength_t>>(64);
    SetOperator setOperator{};
    db->put(SimpleString<memDbDataLength_t>::fromChar(0x41), SimpleString<memDbDataLength_t>::fromChar(0x01), IGNORE_TIMESTAMP, 3, 1);

    //A -> 1
    auto result = setOperator.operate(
            createOperationSet(0x41, 0x02, 2, 1),
            OperationOptions{.checkTimestamps=false},
            OperatorDependencies{.dbStore = db});

    ASSERT_TRUE(result.isSuccessful);
    ASSERT_EQ(* db->get(SimpleString<memDbDataLength_t>::fromChar('A'))._value()._value.data(), 0x02);
}

TEST(SetOperator, ShouldntReplaceNewerKeyTimestamp) { //fails
    memDbDataStore_t db = std::make_shared<Map<memDbDataLength_t>>(64);
    SetOperator setOperator{};
    db->put(SimpleString<memDbDataLength_t>::fromChar(0x41), SimpleString<memDbDataLength_t>::fromChar(0x01), IGNORE_TIMESTAMP, 3, 1);

    //A -> 1
    auto result = setOperator.operate(
            createOperationSet(0x41, 0x02, 2, 1),
            OperationOptions{.checkTimestamps=true},
            OperatorDependencies{.dbStore = db});

    ASSERT_FALSE(result.isSuccessful);
    ASSERT_EQ(* db->get(SimpleString<memDbDataLength_t>::fromChar('A'))._value()._value.data(), 0x01);
    ASSERT_EQ(result.errorCode, ErrorCode::ALREADY_REPLICATED);
}

TEST(SetOperator, ShouldReplaceOldKeyTimestamp) {
    memDbDataStore_t db = std::make_shared<Map<memDbDataLength_t>>(64);
    SetOperator setOperator{};
    db->put(SimpleString<memDbDataLength_t>::fromChar(0x41), SimpleString<memDbDataLength_t>::fromChar(0x01), NOT_IGNORE_TIMESTAMP, 1, 1);

    //A -> 1
    auto result = setOperator.operate(
            createOperationSet(0x41, 0x02, 2, 1),
            OperationOptions{.checkTimestamps=true},
            OperatorDependencies{.dbStore = db});

    ASSERT_TRUE(result.isSuccessful);
    ASSERT_EQ(* db->get(SimpleString<memDbDataLength_t>::fromChar('A'))._value()._value.data(), 0x02);
}

TEST(SetOperator, ShouldSetNewKey) {
    memDbDataStore_t db = std::make_shared<Map<memDbDataLength_t>>(64);
    SetOperator setOperator{};

    //A -> 1
    Response response = setOperator.operate(
            createOperationSet(0x41, 0x01, 1, 1),
            OperationOptions{.checkTimestamps=true},
            OperatorDependencies{.dbStore = db});

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_TRUE(db->contains(SimpleString<memDbDataLength_t>::fromChar('A')));
    ASSERT_EQ(* db->get(SimpleString<memDbDataLength_t>::fromChar('A'))._value()._value.data(), 0x01);
}


OperationBody createOperationSet(uint8_t keyValue, uint8_t valueValue, uint64_t timestamp, uint16_t nodeId) {
    SimpleString key = SimpleString<memDbDataLength_t>::fromChar(keyValue);
    SimpleString value = SimpleString<memDbDataLength_t>::fromChar(valueValue);

    std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>> vector = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
    vector->push_back(key);
    vector->push_back(value);

    return OperationBody(SetOperator::OPERATOR_NUMBER, false, false, timestamp, nodeId, vector);
}
