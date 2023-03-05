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
    ASSERT_EQ(deleteOperator.authorizedToExecute(), AuthenticationType::USER);
}

TEST(DeleteOperator, ShouldntDeleteNewerKeyTimestamp){
    DeleteOperator deleteOperator{};
    memDbDataStore_t db = std::make_shared<Map<defaultMemDbLength_t>>(64);

    SimpleString key = SimpleString<defaultMemDbLength_t>::fromChar(0x41);
    SimpleString value = SimpleString<defaultMemDbLength_t>::fromChar(0x01);
    db->put(key, value, NOT_IGNORE_TIMESTAMP, 2, 1);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 1, 1), OperationOptions{.requestFromReplication = true}, db);

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_TRUE(db->contains(key));
}

TEST(DeleteOperator, ShouldtDeleteEventNewerKeyTimestamp){
    DeleteOperator deleteOperator{};
    memDbDataStore_t db = std::make_shared<Map<defaultMemDbLength_t>>(64);

    SimpleString key = SimpleString<defaultMemDbLength_t>::fromChar(0x41);
    db->put(key, SimpleString<defaultMemDbLength_t>::fromChar(0x01), NOT_IGNORE_TIMESTAMP, 2, 1);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 1, 1), OperationOptions{.requestFromReplication = false}, db);

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_FALSE(db->contains(key));
}

TEST(DeleteOperator, ShouldDeleteOlderKeyTimestamp){
    DeleteOperator deleteOperator{};
    memDbDataStore_t db = std::make_shared<Map<defaultMemDbLength_t>>(64);

    db->put(SimpleString<defaultMemDbLength_t>::fromChar(0x41), SimpleString<defaultMemDbLength_t>::fromChar(0x01), NOT_IGNORE_TIMESTAMP, 1, 1);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 2, 1), OperationOptions{.requestFromReplication = true}, db);

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_FALSE(db->contains(SimpleString<defaultMemDbLength_t>::fromChar(0x41)));
}

TEST(DeleteOperator, KeyNotFound){
    DeleteOperator deleteOperator{};
    memDbDataStore_t db = std::make_shared<Map<defaultMemDbLength_t>>(64);

    auto response = deleteOperator.operate(createOperationDelete(0x41, 1, 2), OperationOptions{.requestFromReplication = true}, db);

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, ErrorCode::UNKNOWN_KEY);
}

OperationBody createOperationDelete(uint8_t keyValue, uint64_t timestamp, uint16_t nodeId){
    SimpleString key = SimpleString<defaultMemDbLength_t>::fromChar(keyValue);

    std::shared_ptr<std::vector<SimpleString<defaultMemDbLength_t>>> vector = std::make_shared<std::vector<SimpleString<defaultMemDbLength_t>>>();
    vector->push_back(key);

    return OperationBody(DeleteOperator::OPERATOR_NUMBER, false, false, timestamp, nodeId, vector);

}
