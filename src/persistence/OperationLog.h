#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "messages/request/Request.h"

struct OperationLog {
    std::shared_ptr<std::vector<SmallString>> args;
    uint8_t operatorNumber;
    bool flag1;
    bool flag2;

    OperationLog(std::shared_ptr<std::vector<SmallString>> args, uint8_t operatorNumber, bool flag1, bool flag2):
        args(args),
        operatorNumber(operatorNumber),
        flag1(flag1),
        flag2(flag2) {}
};