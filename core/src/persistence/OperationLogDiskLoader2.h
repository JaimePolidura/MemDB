#pragma once

#include <memory>
#include <chrono>

#include "utils/files/FileUtils.h"
#include "utils/datastructures/map/Map.h"
#include "OperationLogDeserializer.h"
#include "operators/OperatorDispatcher.h"
#include "messages/request/Request.h"

class OperationLogDiskLoader2 {
private:
    OperationLogDeserializer operationLogDeserializer;

public:
    std::vector<OperationBody> getAll() {
        if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", "oplog")))
            return std::vector<OperationBody>{};

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<OperationBody> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        return logs;
    }

    /**
     * @return The last log's timestamp
     */
    uint64_t loadIntoMapDbAndCompact(memDbDataStore_t db, operatorDisptacher_t operationDispatcher) {
        if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", "oplog")))
            return 0;

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<OperationBody> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        this->executeOperationLogs(db, logs, operationDispatcher);
        this->clearFileAndAddNewCompressedOperations(db);

        return logs[logs.size() - 1].timestamp;
    }

private:
    void executeOperationLogs(memDbDataStore_t db, const std::vector<OperationBody>& operations, operatorDisptacher_t operationDispatcher) {
        printf("[SERVER] Applaying logs...\n");

        for (const auto &operation : operations)
            operationDispatcher->executeOperator(db, OperationOptions{.requestFromReplication = false}, operation);
    }

    void clearFileAndAddNewCompressedOperations(std::shared_ptr<Map<defaultMemDbSize_t>> db) {
        printf("[SERVER] Compacting logs...\n");

        std::vector<MapEntry<defaultMemDbSize_t>> allDataMap = db->all();
        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<uint8_t> toWriteCompressed{};

        for (const auto& entry: allDataMap) { //TODO When comapacting, add as well timestamp. Possible fix: current timestamp
            toWriteCompressed.push_back(SetOperator::OPERATOR_NUMBER << 2);

            toWriteCompressed.push_back(entry.key.size);
            for(std::size_t i = 0; i < entry.key.size; i++) //Key Hash
                toWriteCompressed.push_back(* entry.key[i]);
            toWriteCompressed.push_back(entry.value.size); //Value size
            for(int i = 0; i < entry.value.size; i++) //Value content
                toWriteCompressed.push_back(* (entry.value + i));

            toWriteCompressed.push_back(0x00); //Padding
        }

        FileUtils::clear(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        FileUtils::appendBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"), toWriteCompressed);
    }
};