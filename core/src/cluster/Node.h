#pragma once

#include "shared.h"
#include "cluster/NodeState.h"
#include "server/Connection.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "utils/net/DNSUtils.h"
#include "utils/std/Result.h"
#include "logging/Logger.h"
#include "utils/Utils.h"

struct Node {
public:
    io_context ioContext;
    logger_t logger;

    mutable connection_t connection;
    std::string address;
    NodeState state;
    memdbNodeId_t nodeId;

    uint64_t readTimeout;

    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;

    Node();

    Node(memdbNodeId_t nodeId, const std::string& address, NodeState state, uint64_t readTimeout);

    Node(const Node& other);
    
    auto sendRequest(const Request &request) -> std::result<Response>;

    void closeConnection();

    bool isConnectionOpened() const;

    bool openConnection();

    void setLogger(logger_t logger);

private:
    void openConnectionIfClosedOrThrow();

    std::size_t writeSyncToConnection(std::vector<uint8_t>& bytes);

    std::result<std::vector<uint8_t>> readSyncFromConnection(uint64_t timeoutMs);

public:
    static bool canSendRequestUnicast(NodeState state);

    static std::string toJson(std::shared_ptr<Node> node);

    static std::shared_ptr<Node> fromJson(const nlohmann::json& json);
};

using node_t = std::shared_ptr<Node>;