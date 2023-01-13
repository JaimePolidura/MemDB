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

        uint64_t timestamp = std::chrono::duration_cast
                <std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count();

        this->buffer->add(OperationLog{
            timestamp,
            request.operation.args,
            request.operation.operatorNumber,
            request.operation.flag1,
            request.operation.flag2
        });
    }
};