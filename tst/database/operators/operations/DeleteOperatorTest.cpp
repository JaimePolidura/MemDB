#include "gtest/gtest.h"
#include "messages/request/Request.h"
#include "operators/operations/DeleteOperator.h"

#include <string>
#include <memory>

OperationBody createOperation(uint8_t keyValue, uint64_t timestamp, uint16_t nodeId);

TEST(DeleteOperator, CorrectConfig) {
    DeleteOperator deleteOperator{};

    ASSERT_EQ(deleteOperator.type(), WRITE);
    ASSERT_EQ(deleteOperator.operatorNumber(), DeleteOperator::OPERATOR_NUMBER);
}

TEST(DeleteOperator, ShouldntDeleteNewerKeyTimestamp){
    DeleteOperator deleteOperator{};
    std::shared_ptr<Map> db = std::make_shared<Map>(64);

    db->put(SimpleString::fromChar(0x41), SimpleString::fromChar(0x01), 2, 1);

    auto response = deleteOperator.operate(createOperation(0x41, 1, 1), db);

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_TRUE(db->contains(SimpleString::fromChar(0x41)));
}

TEST(DeleteOperator, ShouldDeleteOlderKeyTimestamp){
    DeleteOperator deleteOperator{};
    std::shared_ptr<Map> db = std::make_shared<Map>(64);

    db->put(SimpleString::fromChar(0x41), SimpleString::fromChar(0x01), 1, 1);

    auto response = deleteOperator.operate(createOperation(0x41, 2, 1), db);

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_FALSE(db->contains(SimpleString::fromChar(0x41)));
}

TEST(DeleteOperator, KeyNotFound){
    DeleteOperator deleteOperator{};
    std::shared_ptr<Map> db = std::make_shared<Map>(64);

    auto response = deleteOperator.operate(createOperation(0x41, 1, 2), db);

    ASSERT_FALSE(!response.isSuccessful);
    ASSERT_EQ(response.errorCode, ErrorCode::UNKNOWN_KEY);

}

OperationBody createOperation(uint8_t keyValue, uint64_t timestamp, uint16_t nodeId){
    SimpleString key = SimpleString::fromChar(keyValue);

    std::shared_ptr<std::vector<SimpleString>> vector = std::make_shared<std::vector<SimpleString>>();
    vector->push_back(key);

    return OperationBody(DeleteOperator::OPERATOR_NUMBER, false, false, timestamp, nodeId, vector);

}