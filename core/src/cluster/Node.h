#pragma once

#include "shared.h"
#include "server/Connection.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "utils/std/Result.h"
#include "logging/Logger.h"
#include "utils/net/DNSUtils.h"

struct Node {
    io_context ioContext;
    logger_t logger;

    mutable connection_t connection;
    std::string address;
    memdbNodeId_t nodeId;

    uint64_t readTimeout;

    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;

    Node();
    
    Node(memdbNodeId_t nodeId, const std::string& address, uint64_t readTimeout);

    Node(const Node& other);
    
    auto sendRequest(const Request &request) -> std::result<Response>;

    void closeConnection();

    bool isConnectionOpened() const;

    bool openConnection();

    void setLogger(logger_t logger);

private:
    std::result<bool> openConnectionIfClosedOrThrow();

    bool connectToSocket(std::string& ip, const std::string& port, ip::tcp::socket& socket);

    std::size_t writeSyncToConnection(std::vector<uint8_t>& bytes);

    std::result<std::vector<uint8_t>> readSyncFromConnection(uint64_t timeoutMs);

};

using node_t = std::shared_ptr<Node>;