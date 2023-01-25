#pragma once

#include <vector>
#include <functional>
#include <mutex>

#include "persistence/OperationLogDiskWriter.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "utils/files/FileUtils.h"
#include "messages/request/Request.h"

class OperationLogBuffer {
private:
    std::shared_ptr<Configuration> configuration;
    std::vector<OperationBody> operations;
    OperationsLogDiskWriter diskWriter;
    std::mutex writeDiskLock;

public:
    OperationLogBuffer(std::shared_ptr<Configuration> configuration): configuration(configuration) {}

    void add(const OperationBody& operation) {
        this->increaseArgsRefCount(operation.args);

        this->operations.push_back(std::move(operation));

        if(this->operations.size() >= this->configuration->get<int>(ConfigurationKeys::PERSISTANCE_WRITE_EVERY)){
            this->writeOperationsToDisk();
        }
    }

private:
    void writeOperationsToDisk() {
        if(!this->writeDiskLock.try_lock())
            return;

        std::vector<OperationBody> copyBuffer;
        this->operations.swap(copyBuffer);

        this->diskWriter.write(copyBuffer);

        this->writeDiskLock.unlock();
    }

    void increaseArgsRefCount(std::shared_ptr<std::vector<SimpleString>> args) {
        for(int i = 0; i < args->size(); i++)
            args->at(i).increaseRefCount();
    }
};