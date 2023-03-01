#include "gtest/gtest.h"

#include "../Assertions.h"
#include "persistence/OperationLogSerializer.h"

TEST(OperationLogSerializer, ShouldSerializeWithArgs) {
    OperationLogSerializer operationLogSerializer{};

    std::vector<uint8_t> expected = {
            0x04,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, //Timestamp
            0x00, 0x00, 0x00, 0x00 //Pading
    };

    std::vector<uint8_t> serialized{};

    operationLogSerializer.serialize(serialized, OperationBody{
            0x01, false, false, 0x02, 1
    });

    Assertions::assertFirstItemsVectorsEqual(expected, serialized);
}

TEST(OperationLogSerializer, ShouldSerializeNoArgs) {
    OperationLogSerializer operationLogSerializer{};

    std::shared_ptr<std::vector<SimpleString<defaultMemDbSize_t>>> args = std::make_shared<std::vector<SimpleString<defaultMemDbSize_t>>>();
    args->push_back(SimpleString<defaultMemDbSize_t>::fromChar('A'));
    args->push_back(SimpleString<defaultMemDbSize_t>::fromChar('B'));

    std::vector<uint8_t> expected = {
            0x04,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
            0x01, 0x41,
            0x01, 0x42,
            0x00
    };

    std::vector<uint8_t> serialized{};

    operationLogSerializer.serialize(serialized, OperationBody{
        0x01, false, false, 0x02, 1, args
    });

    Assertions::assertFirstItemsVectorsEqual(expected, serialized);
}