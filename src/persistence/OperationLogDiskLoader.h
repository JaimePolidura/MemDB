#include <memory>
#include <chrono>

#include "utils/files/FileUtils.h"
#include "utils/datastructures/map/Map.h"

#include "OperationLog.h"
#include "OperationLogDeserializer.h"
#include "operators/OperatorDispatcher.h"

class OperationLogDiskLoader {
private:
    std::shared_ptr<OperatorDispatcher> operationDispatcher;
    OperationLogDeserializer operationLogDeserializer;

public:
    OperationLogDiskLoader(std::shared_ptr<OperatorDispatcher> operationDispatcher) : operationDispatcher(operationDispatcher) {}

    std::shared_ptr<Map> loadIntoMapDb(std::shared_ptr<Map> db) {
        if(!FileUtils::exists(FileUtils::getProgramBasePath("memdb"), "oplog.data"))
            return db;

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog.data"));
        std::vector<OperationLog> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        this->executeOperationLogs(db, logs);

        std::vector<MapEntry> allDataMap = db->all();
        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog.data"));
        std::vector<uint8_t> toWriteCompressed{};
        uint64_t timestamp = std::chrono::duration_cast
                <std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count();

        for (const MapEntry& entry: allDataMap) {
            for(std::size_t i = 0; i < sizeof(uint64_t); i++) //Timestamp
                toWriteCompressed.push_back(static_cast<uint8_t>(timestamp >> (sizeof(uint64_t) * i)));

            toWriteCompressed.push_back(SetOperator::OPERATOR_NUMBER << 2 | 0x02); //Flag 1 true -> pre hashed value TOOD fix store key

            toWriteCompressed.push_back(sizeof(uint32_t));
            for(std::size_t i = 0; i < sizeof(uint32_t); i++) //Key Hash
                toWriteCompressed.push_back(static_cast<uint32_t>(entry.keyHash >> (sizeof(uint32_t) * i)));
            toWriteCompressed.push_back(entry.valueSize); //Value size
            for(int i = 0; i < entry.valueSize; i++) //Value content
                toWriteCompressed.push_back(* (entry.value + i));

            toWriteCompressed.push_back(0x00); //Padding
        }

        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog.data"));
        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog.data"), toWriteCompressed);

        return db;
    }

private:
    void executeOperationLogs(std::shared_ptr<Map> db, const std::vector<OperationLog>& logs) {
        for (const auto &operationLog : logs)
            this->operationDispatcher->executeOperator(db, OperationBody{operationLog.operatorNumber, operationLog.flag1, operationLog.flag2, operationLog.args});

    }

};