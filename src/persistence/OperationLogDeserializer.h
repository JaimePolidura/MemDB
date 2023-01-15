#include <vector>
#include <cstdint>
#include <memory>
#include <numeric>
#include <thread>
#include <chrono>

#include "OperationLog.h"
#include "messages/request/RequestDeserializer.h"

class OperationLogDeserializer {
private:
    RequestDeserializer requestDeserializer;
    
public:
    std::vector<OperationLog> deserializeAll(const std::vector<uint8_t>& bytes) {
        std::vector<OperationLog> logs{};

        int counter = 0;

        uint64_t a = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        for(auto bytesPtr = bytes.begin(); bytesPtr < bytes.end();) {
            const std::vector<uint8_t> buffer1 = std::vector<uint8_t>(bytesPtr, bytes.end() - 1);
            OperationBody operationBody = this->requestDeserializer.deserializeOperation(std::move(buffer1));

            const OperationLog operationLog = OperationLog{operationBody.args, operationBody.operatorNumber,
                                                           operationBody.flag1, operationBody.flag2};
            logs.push_back(operationLog);

            bytesPtr += this->calculateOperationLogSizeInDisk(operationLog);

            counter++;

            if(counter == 1000){
                uint64_t b = std::chrono::duration_cast
                        <std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();

                printf("Deserialized 1000 elements in %i ms\n", b - a);
                counter = 0;
                a = std::chrono::duration_cast
                        <std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();
            }

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
                    [](int acc, const OperatorArgument& it) {
                        return acc + it.lengthArg;
                    });

            return
                    1 + //Operation number
                    totalArgLength + //Args length (value + size)
                    1; //Padding;
        }
    };
};







































