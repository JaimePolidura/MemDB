#pragma once

#include "messages/request/Request.h"
#include "config/Configuration.h"
#include "config/keys/ConfigurationKeys.h"
#include "OperationLogBuffer.h"

#include <memory>
#include <chrono>

class OperationLogSaver {
private:
    std::shared_ptr<Configuration> configuration;
    std::shared_ptr<OperationLogBuffer> buffer;

public:
    OperationLogSaver(std::shared_ptr<Configuration> configuration, std::shared_ptr<OperationLogBuffer> buffer) :
        configuration(configuration), buffer(buffer)
    {}

    void save(const Request& request) {
        this->increaseArgsRefCount(request.operation.args);

        this->buffer->add(OperationLog{
            request.operation.args,
            request.operation.operatorNumber,
            request.operation.flag1,
            request.operation.flag2
        });
    }

private:
    void increaseArgsRefCount(std::shared_ptr<std::vector<SimpleString>> args) {
        for(int i = 0; i < args->size(); i++)
            args->at(i).increaseRefCount();
    }

};