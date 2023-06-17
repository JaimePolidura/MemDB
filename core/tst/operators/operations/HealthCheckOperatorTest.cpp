#include "gtest/gtest.h"

#include "operators/operations/cluster/HealthCheckOperator.h"

TEST(HealthCheckOperator, CorrectConfig) {
    HealthCheckOperator healthCheckOperator{};

    ASSERT_EQ(healthCheckOperator.type(), OperatorType::NODE_MAINTENANCE);
    ASSERT_EQ(healthCheckOperator.operatorNumber(), HealthCheckOperator::OPERATOR_NUMBER);
    ASSERT_EQ(healthCheckOperator.authorizedToExecute(), AuthenticationType::CLUSTER);
}