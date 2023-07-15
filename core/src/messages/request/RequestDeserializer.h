#pragma once

#include "messages/request/Request.h"
#include "utils/Utils.h"
#include "memdbtypes.h"

#include "shared.h"

class RequestDeserializer {
private:
    static const uint8_t FLAG1_MASK = 0x20; //1000 0000
    static const uint8_t FLAG2_MASK = 0x10; //0100 0000

public:
    //In buffer we dont pass total request length
    Request deserialize(const std::vector<uint8_t>& buffer);

    AuthenticationBody deserializeAuthenticacion(const std::vector<uint8_t>& buffer);

    OperationBody deserializeOperation(const std::vector<uint8_t>& buffer, uint64_t position = 0,
                                       const bool includesNodeId = false);

private:
    uint8_t * fill(const std::vector<uint8_t>& buffer, int initialPos, int endPos);

    uint8_t getValueWithoutFlags(const std::vector<uint8_t>& buffer, uint64_t pos);

    bool getFlag(const std::vector<uint8_t>& buffer, uint64_t pos, char flagMask);
};