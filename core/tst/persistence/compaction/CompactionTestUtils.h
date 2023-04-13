#pragma once

#include "messages/request/Request.h"
#include "utils/strings/SimpleString.h"

#include <vector>

class CompactionTestUtils {
public:
    static OperationBody set(char key, char value) {
        OperationBody operationBody{};
        std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>> argsToAddd = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
        argsToAddd->insert(argsToAddd->begin(), SimpleString<memDbDataLength_t>::fromChar(key));
        argsToAddd->insert(argsToAddd->begin() + 1, SimpleString<memDbDataLength_t>::fromChar(value));

        operationBody.operatorNumber = 1;
        operationBody.args = argsToAddd;

        return operationBody;
    }

    static OperationBody del(char key) {
        OperationBody operationBody{};
        std::shared_ptr<std::vector<SimpleString<memDbDataLength_t>>> argsToAddd = std::make_shared<std::vector<SimpleString<memDbDataLength_t>>>();
        argsToAddd->insert(argsToAddd->begin(), SimpleString<memDbDataLength_t>::fromChar(key));

        operationBody.operatorNumber = 2;
        operationBody.args = argsToAddd;

        return operationBody;
    }
};