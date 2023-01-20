#pragma once

#include <vector>
#include <cstdint>
#include <memory>
#include <numeric>
#include <thread>
#include <chrono>

#include "OperationLog.h"
#include "messages/request/RequestDeserializer.h"
#include "utils/benchmark/ScopeTimer.h"

class OperationLogDeserializer {
private:
    RequestDeserializer requestDeserializer;

public:
    std::vector<OperationLog> deserializeAll(const std::vector<uint8_t>& bytes) {
        std::vector<OperationLog> logs{};

        const uint8_t * firstElementPtr = bytes.data();
        const uint8_t * lastElementPtr = &bytes.back();

        for(const uint8_t * bytesPtr = firstElementPtr; bytesPtr < (lastElementPtr + 1);) {
            uint64_t initialOffset = bytesPtr - firstElementPtr;
            OperationBody operationBody = this->requestDeserializer.deserializeOperation(bytes, initialOffset);
            
            const OperationLog operationLog = OperationLog{operationBody.args, operationBody.operatorNumber,
                                                           operationBody.flag1, operationBody.flag2};
            logs.push_back(operationLog);

            uint8_t toIncrease = this->calculateOperationLogSizeInDisk(operationLog);
            bytesPtr = bytesPtr + toIncrease;
        }

        return logs;
    }

private:
    uint8_t calculateOperationLogSizeInDisk(const OperationLog log) const {
        int totalArgLength = std::accumulate(
                log.args->begin(),
                log.args->end(),
                log.args->size(),
                [](int acc, const SmallString& it) {
                    return acc + it.size;
                });

        return
            1 + //Operation number
            totalArgLength + //Args length (value + size)
            1; //Padding;
    }

    class OperationLogDeserializerWorker {
    private:
        RequestDeserializer requestDeserializer;
        std::vector<OperationLog> deserialized;
        std::vector<int8_t> serialized;
        std::thread thread;

    public:
        OperationLogDeserializerWorker(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end): serialized(begin, end)  {}

        void run() {
            this->thread = std::thread([this]{this->deserialize();});
        }

        void join() {
            this->thread.join();
        }

    private:
        void deserialize() {
            for(auto bytesPtr = this->serialized.begin(); bytesPtr < this->serialized.end();) {
                OperationBody operationBody = this->requestDeserializer.deserializeOperation(std::vector<uint8_t>(bytesPtr, this->serialized.end()));

                const OperationLog operationLog = OperationLog{operationBody.args, operationBody.operatorNumber,
                                                               operationBody.flag1, operationBody.flag2};
                this->deserialized.push_back(operationLog);

                bytesPtr += this->calculateOperationLogSizeInDisk(operationLog);
            }
        }

        uint8_t calculateOperationLogSizeInDisk(const OperationLog log) const {
            int totalArgLength = std::accumulate(
                    log.args->begin(),
                    log.args->end(),
                    log.args->size(),
                    [](int acc, const SmallString& it) {
                        return acc + it.size;
                    });

            return
                    1 + //Operation number
                    totalArgLength + //Args length (value + size)
                    1; //Padding;
        }
    };
};