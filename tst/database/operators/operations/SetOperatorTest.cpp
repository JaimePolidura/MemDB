#include "gtest/gtest.h"
#include "messages/request/Request.h"
#include "operators/operations/SetOperator.h"
#include "messages/response/ErrorCode.h"

#include <string>
#include <memory>

OperationBody createOperationSet(uint8_t keyValue, uint8_t valueValue, uint64_t timestamp, uint16_t nodeId);

TEST(SetOperator, CorrectConfig) {
    SetOperator setOperator{};

    ASSERT_EQ(setOperator.type(), WRITE);
    ASSERT_EQ(setOperator.operatorNumber(), SetOperator::OPERATOR_NUMBER);
}

TEST(SetOperator, ShouldntReplaceNewerKeyTimestamp) {
    std::shared_ptr<Map> db = std::make_shared<Map>(64);
    SetOperator setOperator{};
    db->put(SimpleString::fromChar(0x41), SimpleString::fromChar(0x01), 3, 1);

    auto operation = createOperationSet(0x41, 0x02, 2, 1); //A -> 1
    auto result = setOperator.operate(operation, db);

    ASSERT_FALSE(result.isSuccessful);
    ASSERT_EQ(* db->get(SimpleString::fromChar('A')).value().value.value, 0x01);
    ASSERT_EQ(result.errorCode, ErrorCode::ALREADY_REPLICATED);
}

TEST(SetOperator, ShouldReplaceOldKeyTimestamp) {
    std::shared_ptr<Map> db = std::make_shared<Map>(64);
    SetOperator setOperator{};
    db->put(SimpleString::fromChar(0x41), SimpleString::fromChar(0x01), 1, 1);

    auto operation = createOperationSet(0x41, 0x02, 2, 1); //A -> 1
    auto result = setOperator.operate(operation, db);

    ASSERT_TRUE(result.isSuccessful);
    ASSERT_EQ(* db->get(SimpleString::fromChar('A')).value().value.value, 0x02);
}

TEST(SetOperator, ShouldSetNewKey) {
    std::shared_ptr<Map> db = std::make_shared<Map>(64);
    SetOperator setOperator{};
    auto operation = createOperationSet(0x41, 0x01, 1, 1); //A -> 1

    Response response = setOperator.operate(operation, db);

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_TRUE(db->contains(SimpleString::fromChar('A')));
    ASSERT_EQ(* db->get(SimpleString::fromChar('A')).value().value.value, 0x01);
}


OperationBody createOperationSet(uint8_t keyValue, uint8_t valueValue, uint64_t timestamp, uint16_t nodeId) {
    SimpleString key = SimpleString::fromChar(keyValue);
    SimpleString value = SimpleString::fromChar(valueValue);

    std::shared_ptr<std::vector<SimpleString>> vector = std::make_shared<std::vector<SimpleString>>();
    vector->push_back(key);
    vector->push_back(value);

    return OperationBody(SetOperator::OPERATOR_NUMBER, false, false, timestamp, nodeId, vector);
}
