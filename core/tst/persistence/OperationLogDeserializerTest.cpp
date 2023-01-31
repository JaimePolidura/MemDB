#include "gtest/gtest.h"

#include "persistence/OperationLogDeserializer.h"

TEST(OperationLogDeserializer, ShouldDeserialize) {
    OperationLogDeserializer operationLogDeserializer{};

    std::vector<uint8_t> toDesrialize = {
            0x04,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
            0x01,0x41,
            0x01,0x42,

            0x00, //Padding

            0x04,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
            0x01,0x41,
            0x01,0x42,

            0x00,
    };

    std::vector<OperationBody> deserialized = operationLogDeserializer.deserializeAll(toDesrialize);

    ASSERT_TRUE(deserialized.size() == 2);

    auto firstOperation = deserialized.at(0);
    auto secondOperation = deserialized.at(1);

    ASSERT_EQ(firstOperation.timestamp, 255);
    ASSERT_EQ(firstOperation.operatorNumber, 1);
    ASSERT_EQ(firstOperation.args->size(), 2);
    ASSERT_EQ(* firstOperation.args->at(0).value, 0x041);
    ASSERT_EQ(* firstOperation.args->at(1).value, 0x042);

    ASSERT_EQ(secondOperation.timestamp, 256);
    ASSERT_EQ(secondOperation.operatorNumber, 1);
    ASSERT_EQ(secondOperation.args->size(), 2);
    ASSERT_EQ(* secondOperation.args->at(0).value, 0x041);
    ASSERT_EQ(* secondOperation.args->at(1).value, 0x042);
}