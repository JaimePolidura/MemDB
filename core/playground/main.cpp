#include "utils/Utils.h"
#include "messages/response/ResponseDeserializer.h"
#include <cstdint>
#include <cstdio>
#include "messages/response/Response.h"
#include "messages/response/ResponseSerializer.h"
#include "persistence/OperationLogDeserializer.h"

int main() {
    OperationLogDeserializer operationLogDeserializer{};

    std::vector<uint8_t> toDesrialize = {
            0x04, //Op desc
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, //Timestamp
            0x00, 0x00, 0x00, 0x01 ,0x41, //Arg 1 -> A
            0x00, 0x00, 0x00, 0x01, 0x42, //Arg 2 -> B
            0x00, 0x00, 0x00, 0x00,  //Padding

            0x04, //Op desc
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, //Timestamp
            0x00, 0x00, 0x00, 0x01, 0x41, //Arg 1 -> A
            0x00, 0x00, 0x00, 0x01, 0x42, //Arg 2 -> B
            0x00, 0x00, 0x00, 0x00 //Padding
    };

    std::vector<OperationBody> deserialized = operationLogDeserializer.deserializeAll(toDesrialize);

    auto a = 1;
}