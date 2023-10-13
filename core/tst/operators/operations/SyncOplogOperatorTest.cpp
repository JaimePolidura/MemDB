#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "operators/operations/cluster/SyncOplogOperator.h"
#include "persistence/SingleOperationLog.h"
#include "../../cluster/ClusterTest.h"

using ::testing::_;

class OperationLogMock : public SingleOperationLog {
public:
    MOCK_METHOD2(getAfterTimestamp, std::vector<OperationBody>(uint64_t timestamp, const OperationLogOptions options));
};

class PartitionsMock : public Partitions {
public:
    MOCK_METHOD1(getDistance, int (memdbNodeId_t nodeB));
};

MATCHER_P(operatorLogOptionsEquals, expected, "") { return arg.operationLogId == expected; }

/**
 *             Self
 *              |
 *      A   B   C
 *      0   1   2
 *
 *      A ->  C Sync(3, 0)
 *
 *      Node Id A : 1
 *      Node Id B : 2
 *      Node Id C : 3
 */
TEST(SyncDataOperator, PartitionClusterSyncOplog) {
    configuration_t configuration = std::make_shared<Configuration>(std::map<std::string, std::string>{
            {ConfigurationKeys::USE_PARTITIONS, "true"}
    });
    std::shared_ptr<OperationLogMock> operationLogMock = std::make_shared<OperationLogMock>();
    SyncOplogOperator syncOplogOperator{};
    OperationLogDeserializer operationLogDeserializer{};
    std::shared_ptr<PartitionsMock> partitionsMock = std::make_shared<PartitionsMock>();
    cluster_t cluster = std::make_shared<Cluster>();
    ClusterTest clusterTest{cluster};
    clusterTest.setPartitions(partitionsMock);

    args_t args = OperationBody::createOperationBodyArg();
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber<int>(0));
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber<int>(3)); //Timestmap oplog
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber<int>(0)); //Oplog ID to sync of node
    OperationBody operation{};
    operation.args = args;
    operation.nodeId = 1;
    operation.operatorNumber = SyncOplogOperator::OPERATOR_NUMBER;

    ON_CALL(* partitionsMock, getDistance(testing::Eq(1))).WillByDefault(testing::Return(-2));
    ON_CALL(* operationLogMock, getAfterTimestamp(testing::Eq(3), _))
        .WillByDefault(testing::Return(std::vector<OperationBody>{operation}));

    Response response = syncOplogOperator.operate(operation, OperationOptions{}, OperatorDependencies{
            .configuration = configuration,
            .operationLog = operationLogMock,
            .cluster = cluster,
    });

    ASSERT_TRUE(response.isSuccessful);
    std::vector<OperationBody> result = operationLogDeserializer.deserializeAll(response.responseValue.toVector());
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result.at(0).operatorNumber, SyncOplogOperator::OPERATOR_NUMBER);
}

TEST(SyncDataOperator, SimpleClusterSyncOplog) {
    configuration_t configuration = std::make_shared<Configuration>(std::map<std::string, std::string>{
        {ConfigurationKeys::USE_PARTITIONS, "false"}
    });
    std::shared_ptr<OperationLogMock> operationLogMock = std::make_shared<OperationLogMock>();
    SyncOplogOperator syncOplogOperator{};
    OperationLogDeserializer operationLogDeserializer{};

    args_t args = OperationBody::createOperationBodyArg();
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber(0));
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber(1));
    args->push_back(SimpleString<memDbDataLength_t>::fromNumber(2));

    OperationBody operation{};
    operation.args = args;
    operation.operatorNumber = SyncOplogOperator::OPERATOR_NUMBER;

    ON_CALL(* operationLogMock, getAfterTimestamp(testing::Eq(1), _)).WillByDefault(testing::Return(std::vector<OperationBody>{operation}));

    Response response = syncOplogOperator.operate(operation, OperationOptions{}, OperatorDependencies{
            .configuration = configuration,
            .operationLog = operationLogMock
    });

    ASSERT_TRUE(response.isSuccessful);

    std::vector<OperationBody> result = operationLogDeserializer.deserializeAll(response.responseValue.toVector());
    ASSERT_EQ(result.size(), 1);
    ASSERT_EQ(result.at(0).operatorNumber, SyncOplogOperator::OPERATOR_NUMBER);
}

TEST(SyncDataOperator, CorrectConfig) {
    SyncOplogOperator syncDataOperator{};

    ASSERT_EQ(syncDataOperator.type(), OperatorType::NODE_MAINTENANCE);
    ASSERT_EQ(syncDataOperator.operatorNumber(), SyncOplogOperator::OPERATOR_NUMBER);
}

