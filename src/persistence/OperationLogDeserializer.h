#include <vector>
#include <cstdint>
#include <memory>
#include <numeric>

#include "OperationLog.h"
#include "messages/request/RequestDeserializer.h"

class OperationLogDeserializer {
private:
    RequestDeserializer requestDeserializer;

public:
    std::vector<OperationLog> deserializeAll(const std::vector<uint8_t>& bytes) {
        std::vector<OperationLog> logs{};

        for(auto bytesPtr = bytes.begin(); bytesPtr < bytes.end();) {
            uint64_t timestamp = * bytesPtr;

            OperationBody operationBody = this->requestDeserializer.deserializeOperation(std::vector<uint8_t>(bytesPtr + 1, bytes.end() - 1));

            const OperationLog operationLog = OperationLog{timestamp, operationBody.args, operationBody.operatorNumber,
                                                           operationBody.flag1, operationBody.flag2};
            logs.push_back(operationLog);

            bytesPtr += this->calculateOperationLogSizeInDisk(operationLog);
        }

        return logs;
    }

private:
    uint8_t calculateOperationLogSizeInDisk(const OperationLog log) const {
        int totalArgLength = std::accumulate(
                log.args->begin(),
                log.args->end(),
                log.args->size(),
                [](int acc, const OperatorArgument& it) {
                    return acc + it.lengthArg;
                });

        return
            sizeof(log.timestamp) + //Timestamp
            1 + //Operation number
            totalArgLength + //Args length (value + size)
            1; //Padding;
    }
};