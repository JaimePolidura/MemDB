#include "gtest/gtest.h"
#include "database/operators/OperatorRegistry.h"

TEST(OperatorRegistry, GetOperatorFound) {
    OperatorRegistry registry{};

    std::shared_ptr<Operator> operatorFound = registry.get(0x01); //Set operator

    ASSERT_TRUE(operatorFound != nullptr && operatorFound.get() != nullptr);
    ASSERT_TRUE(operatorFound->type() == WRITE);
    ASSERT_TRUE(dynamic_cast<SetOperator *>(operatorFound.get()));
    ASSERT_FALSE(dynamic_cast<GetOperator *>(operatorFound.get()));
}

TEST(OperatorRegistry, GetOperatorNotFound) {
    OperatorRegistry registry{};

    std::shared_ptr<Operator> operatorFound = registry.get(0x99);

    ASSERT_TRUE(operatorFound.get() == nullptr);
}


