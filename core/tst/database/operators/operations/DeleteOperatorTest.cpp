#include "gtest/gtest.h"
#include "messages/request/Request.h"
#include "operators/operations/DeleteOperator.h"
#include "messages/response/ErrorCode.h"

#include <string>
#include <memory>

OperationBody createOperationDelete(uint8_t keyValue, uint64_t timestamp, uint16_t nodeId);

TEST(DeleteOperator, CorrectConfig) {
    DeleteOperator deleteOperator{};

    ASSERT_EQ(deleteOperator.type(), WRITE);
    ASSERT_EQ(deleteOperator.operatorNumber(), DeleteOperator::OPERATOR_NUMBER);
}

TEST(DeleteOperator, ShouldntDeleteNewerKeyTimestamp){
    DeleteOperator deleteOperator{};
    std::shared_ptr<Map> db = std::make_shared<Map>(64);

    SimpleString key = SimpleString::fromChar(0x41);
    SimpleString value = SimpleString::fromChar(0x01);
    db->put(key, value, NOT_IGNORE_TIMESTAMP, 2, 1);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 1, 1), OperationOptions{.requestFromReplication = true}, db);

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_TRUE(db->contains(key));
}

TEST(DeleteOperator, ShouldtDeleteEventNewerKeyTimestamp){
    DeleteOperator deleteOperator{};
    std::shared_ptr<Map> db = std::make_shared<Map>(64);

    SimpleString key = SimpleString::fromChar(0x41);
    db->put(key, SimpleString::fromChar(0x01), NOT_IGNORE_TIMESTAMP, 2, 1);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 1, 1), OperationOptions{.requestFromReplication = false}, db);

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_FALSE(db->contains(key));
}

TEST(DeleteOperator, ShouldDeleteOlderKeyTimestamp){
    DeleteOperator deleteOperator{};
    std::shared_ptr<Map> db = std::make_shared<Map>(64);

    db->put(SimpleString::fromChar(0x41), SimpleString::fromChar(0x01), NOT_IGNORE_TIMESTAMP, 1, 1);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 2, 1), OperationOptions{.requestFromReplication = true}, db);

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_FALSE(db->contains(SimpleString::fromChar(0x41)));
}

TEST(DeleteOperator, KeyNotFound){
    DeleteOperator deleteOperator{};
    std::shared_ptr<Map> db = std::make_shared<Map>(64);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 1, 2), OperationOptions{.requestFromReplication = true}, db);

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, ErrorCode::UNKNOWN_KEY);
}

OperationBody createOperationDelete(uint8_t keyValue, uint64_t timestamp, uint16_t nodeId){
    SimpleString key = SimpleString::fromChar(keyValue);

    std::shared_ptr<std::vector<SimpleString>> vector = std::make_shared<std::vector<SimpleString>>();
    vector->push_back(key);

    return OperationBody(DeleteOperator::OPERATOR_NUMBER, false, false, timestamp, nodeId, vector);

}
