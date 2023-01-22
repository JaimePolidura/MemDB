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
        if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", "oplog")))
            return db;

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<OperationLog> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        this->executeOperationLogs(db, logs);
        this->clearFileAndAddNewCompressedOperations(db);

        return db;
    }

private:
    void executeOperationLogs(std::shared_ptr<Map> db, const std::vector<OperationLog>& logs) {
        printf("[SERVER] Applaying logs...\n");

        for (const auto &operationLog : logs)
            this->operationDispatcher->executeOperator(db, OperationBody{operationLog.operatorNumber, operationLog.flag1, operationLog.flag2, operationLog.args});
    }

    void clearFileAndAddNewCompressedOperations(std::shared_ptr<Map> db) {
        printf("[SERVER] Compacting logs...\n");

        std::vector<MapEntry> allDataMap = db->all();
        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<uint8_t> toWriteCompressed{};

        for (const MapEntry& entry: allDataMap) {
            toWriteCompressed.push_back(SetOperator::OPERATOR_NUMBER << 2);

            toWriteCompressed.push_back(entry.key.size);
            for(std::size_t i = 0; i < entry.key.size; i++) //Key Hash
                toWriteCompressed.push_back(* entry.key[i]);
            toWriteCompressed.push_back(entry.value.size); //Value size
            for(int i = 0; i < entry.value.size; i++) //Value content
                toWriteCompressed.push_back(* (entry.value.value + i));

            toWriteCompressed.push_back(0x00); //Padding
        }

        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"), toWriteCompressed);
    }
};