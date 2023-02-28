#include "gtest/gtest.h"
#include "operators/operations/SyncDataOperator.h"

TEST(SyncDataOperator, CorrectConfig) {
    SyncDataOperator syncDataOperator{};
    //TODO Pending
    ASSERT_EQ(syncDataOperator.type(), OperatorType::CONTROL);
    ASSERT_EQ(syncDataOperator.operatorNumber(), SyncDataOperator::OPERATOR_NUMBER);
    ASSERT_EQ(syncDataOperator.authorizedToExecute(), AuthenticationType::CLUSTER);
}

