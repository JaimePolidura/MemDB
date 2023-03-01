#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "operators/OperatorDispatcher.h"

class OperatorRegistryMock : public OperatorRegistry {
public:
    MOCK_METHOD1(get, std::shared_ptr<Operator>(uint8_t));
};

class OperationLogBufferMock : public OperationLogBuffer {
public:
    OperationLogBufferMock(configuration_t configuration): OperationLogBuffer(configuration) {}

    MOCK_METHOD1(add, void(const OperationBody&));
};

Request createRequest(uint8_t opNumber);

TEST(OperatorDispatcher, OperatorNotFound) {
    std::shared_ptr<Configuration> configuration = std::make_shared<Configuration>();
    std::shared_ptr<OperationLogBufferMock> operationLogBufferMock = std::make_shared<OperationLogBufferMock>(configuration);
    std::shared_ptr<OperatorRegistryMock> operatorRegistryMock = std::make_shared<OperatorRegistryMock>();

    EXPECT_CALL(* operatorRegistryMock.get(), get(1)).WillOnce(testing::Return(nullptr));

    OperatorDispatcher dispatcher{
        std::make_shared<Map<defaultMemDbSize_t>>(64),
        std::make_shared<LamportClock>(1),
        operationLogBufferMock,
        operatorRegistryMock
    };

    auto response = dispatcher.dispatch(createRequest(1));

    ASSERT_FALSE(response.isSuccessful);
    ASSERT_EQ(response.errorCode, ErrorCode::UNKNOWN_OPERATOR);
}

Request createRequest(uint8_t opNumber) {
    Request request{};
    OperationBody operationBody;
    operationBody.operatorNumber = opNumber;
    request.operation = operationBody;

    return request;
}