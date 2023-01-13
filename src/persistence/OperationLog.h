#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "messages/request/Request.h"

struct OperationLog {
    uint64_t timestamp;
    std::shared_ptr<std::vector<OperatorArgument>> args;
    uint8_t operatorNumber;
    bool flag1;
    bool flag2;

    OperationLog(uint64_t timestamp, std::shared_ptr<std::vector<OperatorArgument>> args, uint8_t operatorNumber, bool flag1, bool flag2):
        timestamp(timestamp),
        args(args),
        operatorNumber(operatorNumber),
        flag1(flag1),
        flag2(flag2) {}
};