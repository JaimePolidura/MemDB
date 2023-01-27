#pragma once

#include <vector>
#include <cstdint>

#include "messages/request/Request.h"
#include "utils/Utils.h"

class OperationLogSerializer {
public:
    std::vector<uint8_t> serialize(std::vector<uint8_t>& serializedOut, const OperationBody& toDeserialize) {
        //Operator number
        serializedOut.push_back(toDeserialize.operatorNumber << 2 | toDeserialize.flag1 << 1 | toDeserialize.flag2);

        //Timestamp
        Utils::parseToBuffer(toDeserialize.timestamp, serializedOut, 1);

        if(toDeserialize.args.get() != nullptr){
            //Args
            for(auto arg = toDeserialize.args->begin(); arg < toDeserialize.args->end(); arg++){
                serializedOut.push_back(arg->size);

                for(uint8_t j = 0; j < arg->size; j++)
                    serializedOut.push_back(* (arg->value + j));
            }
        }

        //Padding
        serializedOut.push_back(0x00);

        return serializedOut;
    }
};