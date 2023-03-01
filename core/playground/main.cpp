#include "utils/Utils.h"
#include "messages/response/ResponseDeserializer.h"
#include <cstdint>
#include <cstdio>

int main() {
    ResponseDeserializer responseDeserializer{};

    auto responseDeserialized = responseDeserializer.deserialize({
                                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, //Request number
                                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Timestamp
                                                                         0x02, //error errorcode 1
                                                                         0x00, 0x00, 0x00, 0x00, //Length response
                                                                 });

    auto a = 1;
}