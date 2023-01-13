#pragma once

#include <vector>
#include <functional>
#include <mutex>

#include "OperationLogDiskWriter.h"
#include "config/Configuration.h"
#include "config/keys/ConfiguartionKeys.h"
#include "utils/files/FileUtils.h"
#include "OperationLog.h"

class OperationLogBuffer {
private:
    std::shared_ptr<Configuration> configuration;
    std::vector<OperationLog> operations;
    OperationsLogDiskWriter diskWriter;
    std::mutex writeDiskLock;

public:
    void add(const OperationLog& operation) {
        this->operations.push_back(std::move(operation));

        if(this->operations.size() >= this->configuration->get<int>(ConfiguartionKeys::PERSISTANCE_WRITE_EVERY)){
            this->writeOperationsToDisk();
        }
    }

    void writeOperationsToDisk() {
        if(!this->writeDiskLock.try_lock())
            return;

        std::vector<OperationLog> copyBuffer;
        this->operations.swap(copyBuffer);

        this->diskWriter.write(copyBuffer);

        this->writeDiskLock.unlock();
    }
};