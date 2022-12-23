#include "gtest/gtest.h"
#include "database/operators/SetOperator.h"

#include <string>
#include <memory>

OperationBody createOperation(uint8_t keyValue, uint8_t valueValue);

TEST(SetOperator, CorrectConfig) {
    SetOperator setOperator{};

    ASSERT_EQ(setOperator.type(), WRITE);
    ASSERT_EQ(setOperator.operatorNumber(), 0x01);
}

TEST(SetOperator, ShouldSet) {
    std::shared_ptr<Map> db = std::make_shared<Map>();

    SetOperator setOperator{};

    OperationBody operation = createOperation(0x41, 0x01); //A -> 1

    setOperator.operate(operation, db);

    ASSERT_TRUE(db->contains("A"));
    ASSERT_EQ(* db->get("A").value().value, 0x01);
}


OperationBody createOperation(uint8_t keyValue, uint8_t valueValue) {
    uint8_t * key = new uint8_t[1]();
    key[0] = keyValue;
    uint8_t * value = new uint8_t[1]();
    value[0] = valueValue;

    std::vector<OperatorArgument> vector{};
    vector.emplace_back(key, 1);
    vector.emplace_back(value, 1);

    return OperationBody(0, false, false, std::move(vector), 2);
}
