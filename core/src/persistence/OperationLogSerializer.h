#pragma once

#include <vector>

#include "messages/request/Request.h"
#include "utils/Utils.h"

class OperationLogSerializer {
public:
    std::vector<uint8_t> serialize(std::vector<uint8_t>& serializedOut, const OperationBody& toDeserialize) {
        //Operator number
        serializedOut.push_back(toDeserialize.operatorNumber << 2 | toDeserialize.flag1 << 1 | toDeserialize.flag2);

        //Timestamp
        Utils::appendToBuffer(toDeserialize.timestamp, serializedOut);

        if(toDeserialize.args.get() != nullptr){
            //Args
            for(auto arg = toDeserialize.args->begin(); arg < toDeserialize.args->end(); arg++){
                Utils::appendToBuffer(arg->size, serializedOut);

                for(uint8_t j = 0; j < arg->size; j++)
                    serializedOut.push_back(* (arg->data() + j));
            }
        }

        writePadding(serializedOut);

        return serializedOut;
    }

private:
    void writePadding(std::vector<uint8_t>& buffer) {
        for(int i = 0; i< sizeof(defaultMemDbLength_t); i++){
            buffer.push_back(0x00);
        }
    }
};