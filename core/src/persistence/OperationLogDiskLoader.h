#pragma once

#include "utils/files/FileUtils.h"
#include "utils/datastructures/map/Map.h"
#include "OperationLogDeserializer.h"
#include "operators/operations/SetOperator.h"


class OperationLogDiskLoader {
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
    std::vector<OperationBody> getAllAndSaveCompacted(memDbDataStore_t db) {
        if(!FileUtils::exists(FileUtils::getFileInProgramBasePath("memdb", "oplog")))
            return std::vector<OperationBody>{};

        std::vector<uint8_t> bytesFromOpLog = FileUtils::readBytes(FileUtils::getFileInProgramBasePath("memdb", "oplog"));
        std::vector<OperationBody> logs = this->operationLogDeserializer.deserializeAll(bytesFromOpLog);

        this->clearFileAndAddNewCompressedOperations(db);

        return logs;
    }

private:
    void clearFileAndAddNewCompressedOperations(memDbDataStore_t db) {
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