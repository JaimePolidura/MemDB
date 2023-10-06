#include "gtest/gtest.h"

#include "../Assertions.h"
#include "persistence/utils/OperationLogSerializer.h"

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

    std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>> args = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
    args->push_back(SimpleString<memDbDataLength_t>::fromChar('A'));
    args->push_back(SimpleString<memDbDataLength_t>::fromChar('B'));

    std::vector<uint8_t> expected = {
            0x04, //Op desc
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, //Timestamp
            0x00, 0x00, 0x00, 0x01, 0x41, //Arg 1
            0x00, 0x00, 0x00, 0x01, 0x42, //Arg 2
            0x00, 0x00, 0x00, 0x00 //Padding
    };

    std::vector<uint8_t> serialized{};

    operationLogSerializer.serialize(serialized, OperationBody{
        0x01, false, false, 0x02, 1, args
    });

    Assertions::assertFirstItemsVectorsEqual(expected, serialized);
}