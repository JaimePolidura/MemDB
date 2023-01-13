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
    void save(const Request& request) {
        if(!configuration->getBoolean(ConfiguartionKeys::USE_PERSISTENCE))
            return;

        uint64_t timestamp = std::chrono::duration_cast
                <std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count();

        OperationLog operationLog{timestamp, request.operation.args, request.operation.operatorNumber};

        this->buffer->add(operationLog);
    }
};