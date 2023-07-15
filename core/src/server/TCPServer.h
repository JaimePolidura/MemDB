#pragma once

#include "shared.h"

#include "config/keys/ConfigurationKeys.h"
#include "config/Configuration.h"
#include "auth/Authenticator.h"
#include "messages/request/RequestDeserializer.h"
#include "messages/response/ResponseSerializer.h"
#include "messages/response/ErrorCode.h"
#include "utils/threads/pool/DynamicThreadPool.h"
#include "server/Connection.h"
#include "operators/OperatorDispatcher.h"
#include "logging/Logger.h"

using namespace boost::asio;

class TCPServer {
private:
    operatorDispatcher_t operatorDispatcher;
    configuration_t configuration;
    DynamicThreadPool connectionThreadPool;
    uint16_t port;
    RequestDeserializer requestDeserializer;
    ResponseSerializer responseSerializer;
    Authenticator authenicator;
    io_context ioContext;
    ip::tcp::acceptor acceptator;
    logger_t logger;

public:
    TCPServer(logger_t logger, configuration_t configuration, Authenticator authenicator, operatorDispatcher_t operatorDispatcher);

    void run();

private:
    void acceptNewConnections();

    void onNewRequest(const std::vector<uint8_t>& requestRawBuffer, connection_t connection);

    void sendAuthError(connection_t connection, Request request);

    void sendResponse(std::shared_ptr<Connection> connection, const Response& response);
};

using tcpServer_t = std::shared_ptr<TCPServer>;