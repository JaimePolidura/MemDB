#include <vector>
#include <cstdint>

#include "OperationLog.h"

class OperationLogSerializer {
public:
    std::vector<uint8_t> serialize(std::vector<uint8_t>& serializedOut, const OperationLog& toDeserialize) {
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