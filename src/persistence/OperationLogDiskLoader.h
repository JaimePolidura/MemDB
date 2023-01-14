#include <memory>

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

        for (const auto &operationLog : logs)
            this->operationDispatcher->executeOperator(db, OperationBody{operationLog.operatorNumber, operationLog.flag1, operationLog.flag2, operationLog.args});

        return db;
    }


};