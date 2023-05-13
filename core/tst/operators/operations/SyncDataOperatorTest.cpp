#include "gtest/gtest.h"
#include "operators/operations/user/SyncOplogOperator.h"

TEST(SyncDataOperator, CorrectConfig) {
    SyncOplogOperator syncDataOperator{};
    //TODO Pending
    ASSERT_EQ(syncDataOperator.type(), OperatorType::NODE_MAINTENANCE);
    ASSERT_EQ(syncDataOperator.operatorNumber(), SyncOplogOperator::OPERATOR_NUMBER);
    ASSERT_EQ(syncDataOperator.authorizedToExecute(), AuthenticationType::CLUSTER);
}

