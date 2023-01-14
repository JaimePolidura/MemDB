#include <vector>
#include <cstdint>

#include "OperationLog.h"

class OperationLogSerializer {
public:
    std::vector<uint8_t> serialize(std::vector<uint8_t>& serializedOut, const OperationLog& toDeserialize) {
        for(std::size_t i = 0; i < sizeof(uint64_t); i++) //Timestamp
            serializedOut.push_back(static_cast<uint8_t>(toDeserialize.timestamp >> (sizeof(uint64_t) * i)));

        //Operator number
        serializedOut.push_back(toDeserialize.operatorNumber << 2 | toDeserialize.flag1 << 1 | toDeserialize.flag2);

        //Args
        for(auto arg = toDeserialize.args->begin(); arg < toDeserialize.args->end(); arg++){
            serializedOut.push_back(arg->lengthArg);

            for(uint8_t j = 0; j < arg->lengthArg; j++)
                serializedOut.push_back(* (arg->arg + j));
        }

        //Padding
        serializedOut.push_back(0x00);

        return serializedOut;
    }
};