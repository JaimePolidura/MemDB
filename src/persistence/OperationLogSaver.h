#pragma once

#include "messages/request/Request.h"
#include "config/Configuration.h"
#include "config/keys/ConfiguartionKeys.h"
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
        if(!configuration->getBoolean(ConfiguartionKeys::USE_PERSISTENCE))
            return;

        this->buffer->add(OperationLog{
            request.operation.args,
            request.operation.operatorNumber,
            request.operation.flag1,
            request.operation.flag2
        });
    }
};