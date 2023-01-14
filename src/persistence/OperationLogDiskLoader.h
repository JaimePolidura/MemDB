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
        if(!FileUtils::exists(FileUtils::getProgramBasePath("memdb"), "oplog"))
            return db;

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<OperationLog> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        this->executeOperationLogs(db, logs);
        this->clearFileAndAddNewCompressedOperations(db);

        return db;
    }

private:
    void clearFileAndAddNewCompressedOperations(std::shared_ptr<Map> db) {
        std::vector<MapEntry> allDataMap = db->all();
        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<uint8_t> toWriteCompressed{};

        for (const MapEntry& entry: allDataMap) {
            toWriteCompressed.push_back(SetOperator::OPERATOR_NUMBER << 2 | 0x02); //Flag 1 true -> pre hashed value TOOD fix store key

            toWriteCompressed.push_back(sizeof(uint32_t));
            for(std::size_t i = 0; i < sizeof(uint32_t); i++) //Key Hash
                toWriteCompressed.push_back(static_cast<uint32_t>(entry.keyHash >> (sizeof(uint32_t) * i)));
            toWriteCompressed.push_back(entry.valueSize); //Value size
            for(int i = 0; i < entry.valueSize; i++) //Value content
                toWriteCompressed.push_back(* (entry.value + i));

            toWriteCompressed.push_back(0x00); //Padding
        }

        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"), toWriteCompressed);
    }

    void executeOperationLogs(std::shared_ptr<Map> db, const std::vector<OperationLog>& logs) {
        for (const auto &operationLog : logs)
            this->operationDispatcher->executeOperator(db, OperationBody{operationLog.operatorNumber, operationLog.flag1, operationLog.flag2, operationLog.args});
    }

};