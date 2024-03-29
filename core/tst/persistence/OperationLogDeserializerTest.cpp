#include "gtest/gtest.h"

#include "persistence/serializers/OperationLogDeserializer.h"

TEST(OperationLogDeserializer, ShouldDeserialize) {
    OperationLogDeserializer operationLogDeserializer{};

    std::vector<uint8_t> toDesrialize = {
            0x04, //Op desc
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, //Timestamp
            0x00, 0x00, 0x00, 0x01 ,0x41, //Arg 1 -> A
            0x00, 0x00, 0x00, 0x01, 0x42, //Arg 2 -> B
            0x00, 0x00, 0x00, 0x00, //Padding

            0x04, //Op desc
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, //Timestamp
            0x00, 0x00, 0x00, 0x01, 0x41, //Arg 1 -> A
            0x00, 0x00, 0x00, 0x01, 0x42, //Arg 2 -> B
            0x00, 0x00, 0x00, 0x00 //Padding
    };

    std::vector<OperationBody> deserialized = operationLogDeserializer.deserializeAll(toDesrialize);

    ASSERT_TRUE(deserialized.size() == 2);

    auto firstOperation = deserialized.at(0);
    auto secondOperation = deserialized.at(1);

    ASSERT_EQ(firstOperation.timestamp, 255);
    ASSERT_EQ(firstOperation.operatorNumber, 1);
    ASSERT_EQ(firstOperation.args->size(), 2);
    ASSERT_EQ(* firstOperation.args->at(0).data(), 0x041);
    ASSERT_EQ(* firstOperation.args->at(1).data(), 0x042);

    ASSERT_EQ(secondOperation.timestamp, 256);
    ASSERT_EQ(secondOperation.operatorNumber, 1);
    ASSERT_EQ(secondOperation.args->size(), 2);
    ASSERT_EQ(* secondOperation.args->at(0).data(), 0x041);
    ASSERT_EQ(* secondOperation.args->at(1).data(), 0x042);
}