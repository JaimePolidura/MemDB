#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "operators/OperatorDispatcher.h"

using ::testing::_;

class ConfigurationMock : public Configuration {
public:
    ConfigurationMock() = default;

    MOCK_METHOD1(getBoolean, bool(const std::string&));
};

class ReplicationMock : public Replication {
public:
    ReplicationMock(configuration_t configuration, clusterManagerService_t clusterManager, InfoNodeResponse infoNodeResponse) :
            Replication(configuration, clusterManager, infoNodeResponse)
    {}

    ReplicationMock(configuration_t configuration): Replication(configuration) {}

    MOCK_METHOD0(getNodeState, NodeState());

    MOCK_METHOD1(broadcast, void(const Request&));
};

class LamportClockMock : public LamportClock {
public:
    LamportClockMock(uint16_t nodeId) : LamportClock(nodeId) {}

    MOCK_METHOD1(tick, uint64_t(uint64_t));
};

class OperatorRegistryMock : public OperatorRegistry {
public:
    MOCK_METHOD1(get, std::shared_ptr<Operator>(uint8_t));
};

class OperationLogBufferMock : public OperationLogBuffer {
public:
    OperationLogBufferMock(configuration_t configuration): OperationLogBuffer(configuration) {}

    MOCK_METHOD1(add, void(const OperationBody&));
};

class OperatorDbMockAdapter : public Operator, public DbOperatorExecutor {
public:
    OperatorDbMockAdapter() = default;

    Response operate(const OperationBody& operation, const OperationOptions& operationOptions, memDbDataStore_t map) {
        return Response::success();
    }

    AuthenticationType authorizedToExecute() {
        return AuthenticationType::USER;
    }

    constexpr uint8_t operatorNumber() {
        return 1;
    }

    constexpr OperatorType type()  {
        return OperatorType::WRITE;
    }
};

class OperatorDbMock : public OperatorDbMockAdapter {
public:
    MOCK_METHOD0(type, OperatorType());
    MOCK_METHOD0(operatorNumber, uint8_t());
    MOCK_METHOD0(authorizedToExecute, AuthenticationType());

    MOCK_METHOD3(operate, Response(const OperationBody&, const OperationOptions&, memDbDataStore_t));
};

Request createRequest(uint8_t opNumber, AuthenticationType authType = AuthenticationType::USER, uint64_t timestamp = 0);

TEST(OperatorDispatcher, SuccessfulWriteTypeReplication) {
    uint64_t timestampToUpdate = 2;
    auto request = createRequest(1, AuthenticationType::CLUSTER, timestampToUpdate);
    std::shared_ptr<ConfigurationMock> configuration = std::make_shared<ConfigurationMock>();
    std::shared_ptr<OperationLogBufferMock> operationLogBufferMock = std::make_shared<OperationLogBufferMock>(configuration);
    std::shared_ptr<OperatorRegistryMock> operatorRegistryMock = std::make_shared<OperatorRegistryMock>();
    std::shared_ptr<LamportClockMock> lamportClockMock = std::make_shared<LamportClockMock>(0);
    std::shared_ptr<ReplicationMock> replicationMock = std::make_shared<ReplicationMock>(configuration);

    std::shared_ptr<OperatorDbMock> operatorMock = std::make_shared<OperatorDbMock>();

    ON_CALL(* replicationMock, getNodeState()).WillByDefault(testing::Return(NodeState::RUNNING));
    ON_CALL(* operatorRegistryMock, get(testing::Eq(1))).WillByDefault(testing::Return(operatorMock));
    ON_CALL(* operatorMock, type()).WillByDefault(testing::Return(OperatorType::WRITE));
    ON_CALL(* operatorMock, authorizedToExecute()).WillByDefault(testing::Return(AuthenticationType::CLUSTER));
    ON_CALL(* operatorMock, operate(_, _, _)).WillByDefault(testing::Return(Response::success()));
    ON_CALL(* lamportClockMock, tick(_)).WillByDefault(testing::Return(3));
    ON_CALL(* configuration, getBoolean(testing::Eq(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION))).WillByDefault(testing::Return(true));

    OperatorDispatcher dispatcher{
            std::make_shared<Map<defaultMemDbLength_t>>(64),
            lamportClockMock,
            operationLogBufferMock,
            operatorRegistryMock,
            replicationMock,
            configuration
    };

    EXPECT_CALL(* operationLogBufferMock, add(testing::Eq(request.operation))).Times(1);
    auto response = dispatcher.dispatch(request);

    ASSERT_TRUE(response.isSuccessful);
}

TEST(OperatorDispatcher, SuccessfulWriteNotReplication) {
    uint64_t timestampToUpdate = 2;
    auto request = createRequest(1, AuthenticationType::USER, timestampToUpdate);
    std::shared_ptr<ConfigurationMock> configuration = std::make_shared<ConfigurationMock>();
    std::shared_ptr<OperationLogBufferMock> operationLogBufferMock = std::make_shared<OperationLogBufferMock>(configuration);
    std::shared_ptr<OperatorRegistryMock> operatorRegistryMock = std::make_shared<OperatorRegistryMock>();
    std::shared_ptr<LamportClockMock> lamportClockMock = std::make_shared<LamportClockMock>(0);
    std::shared_ptr<ReplicationMock> replicationMock = std::make_shared<ReplicationMock>(configuration);

    std::shared_ptr<OperatorDbMock> operatorMock = std::make_shared<OperatorDbMock>();

    ON_CALL(* operatorRegistryMock, get(testing::Eq(1))).WillByDefault(testing::Return(operatorMock));
    ON_CALL(* operatorMock, type()).WillByDefault(testing::Return(OperatorType::WRITE));
    ON_CALL(* operatorMock, operate(_, _, _)).WillByDefault(testing::Return(Response::success()));
    ON_CALL(* lamportClockMock, tick(_)).WillByDefault(testing::Return(3));
    ON_CALL(* configuration, getBoolean(testing::Eq(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION))).WillByDefault(testing::Return(false));

    OperatorDispatcher dispatcher{
            std::make_shared<Map<defaultMemDbLength_t>>(64),
            lamportClockMock,
            operationLogBufferMock,
            operatorRegistryMock,
            replicationMock,
            configuration
    };

    EXPECT_CALL(* operationLogBufferMock, add(testing::Eq(request.operation))).Times(1);
    EXPECT_CALL(* lamportClockMock, tick(testing::Eq(2))).Times(1);

    auto response = dispatcher.dispatch(request);

    ASSERT_TRUE(response.isSuccessful);
    ASSERT_EQ(response.timestamp, 3);
}

TEST(OperatorDispatcher, UnsuccessfulOperationExecutor) {
    std::shared_ptr<ConfigurationMock> configuration = std::make_shared<ConfigurationMock>();
    std::shared_ptr<OperationLogBufferMock> operationLogBufferMock = std::make_shared<OperationLogBufferMock>(configuration);
    std::shared_ptr<OperatorRegistryMock> operatorRegistryMock = std::make_shared<OperatorRegistryMock>();
    std::shared_ptr<LamportClockMock> lamportClockMock = std::make_shared<LamportClockMock>(0);
    std::shared_ptr<ReplicationMock> replicationMock = std::make_shared<ReplicationMock>(configuration);
    std::shared_ptr<OperatorDbMock> operatorMock = std::make_shared<OperatorDbMock>();

    ON_CALL(* operatorRegistryMock, get(testing::Eq(1))).WillByDefault(testing::Return(operatorMock));
    ON_CALL(* operatorMock, operate(_, _, _)).WillByDefault(testing::Return(Response::error(ErrorCode::UNKNOWN_KEY)));
    ON_CALL(* configuration, getBoolean(testing::Eq(ConfigurationKeys::MEMDB_CORE_USE_REPLICATION))).WillByDefault(testing::Return(false));

    OperatorDispatcher dispatcher{
            std::make_shared<Map<defaultMemDbLength_t>>(64),
            lamportClockMock,
            operationLogBufferMock,
            operatorRegistryMock,
            replicationMock,
            configuration
    };

    auto response = dispatcher.dispatch(createRequest(1));

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, ErrorCode::UNKNOWN_KEY); //Random error
    EXPECT_CALL(* operationLogBufferMock, add(_)).Times(0);
    EXPECT_CALL(* lamportClockMock, tick(_)).Times(0);
}

TEST(OperatorDispatcher, NotAuthrozied) {
    std::shared_ptr<Configuration> configuration = std::make_shared<Configuration>();
    std::shared_ptr<OperationLogBuffer> operationLogBufferMock = std::make_shared<OperationLogBuffer>(configuration);
    std::shared_ptr<OperatorRegistry> operatorRegistryMock = std::make_shared<OperatorRegistry>();
    std::shared_ptr<ReplicationMock> replicationMock = std::make_shared<ReplicationMock>(configuration);

    OperatorDispatcher dispatcher{
            std::make_shared<Map<defaultMemDbLength_t>>(64),
            std::make_shared<LamportClock>(1),
            operationLogBufferMock,
            operatorRegistryMock,
            replicationMock,
            configuration
    };

    auto response = dispatcher.dispatch(createRequest(1, AuthenticationType::CLUSTER)); //Set operator

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, ErrorCode::NOT_AUTHORIZED);
}

TEST(OperatorDispatcher, OperatorNotFound) {
    std::shared_ptr<Configuration> configuration = std::make_shared<Configuration>();
    std::shared_ptr<OperationLogBufferMock> operationLogBufferMock = std::make_shared<OperationLogBufferMock>(configuration);
    std::shared_ptr<OperatorRegistryMock> operatorRegistryMock = std::make_shared<OperatorRegistryMock>();
    std::shared_ptr<ReplicationMock> replicationMock = std::make_shared<ReplicationMock>(configuration);

    ON_CALL(* operatorRegistryMock, get(testing::Eq(1))).WillByDefault(testing::Return(nullptr));

    OperatorDispatcher dispatcher{
        std::make_shared<Map<defaultMemDbLength_t>>(64),
        std::make_shared<LamportClock>(1),
        operationLogBufferMock,
        operatorRegistryMock,
        replicationMock,
        configuration
    };

    auto response = dispatcher.dispatch(createRequest(1));

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, ErrorCode::UNKNOWN_OPERATOR);

    operatorRegistryMock.reset();
}

Request createRequest(uint8_t opNumber, AuthenticationType authType, uint64_t timetamp) {
    Request request{};
    OperationBody operationBody;
    operationBody.operatorNumber = opNumber;
    request.operation = operationBody;
    request.authenticationType = authType;
    request.operation.timestamp = timetamp;

    return request;
}