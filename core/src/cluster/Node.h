#pragma once

#include "shared.h"
#include "cluster/NodeState.h"
#include "server/Connection.h"
#include "messages/request/RequestSerializer.h"
#include "messages/response/ResponseDeserializer.h"
#include "utils/net/DNSUtils.h"
#include "logging/Logger.h"
#include "utils/Utils.h"

struct Node {
public:
    io_context ioContext;

    mutable connection_t connection;
    std::string address;
    NodeState state;
    memdbNodeId_t nodeId;

    ResponseDeserializer responseDeserializer;
    RequestSerializer requestSerializer;

    Node();

    Node(memdbNodeId_t nodeId, const std::string& address, NodeState state);

    Node(const Node& other);

    auto sendRequest(const Request &request) -> std::optional<Response>;

    void closeConnection();

    bool isConnectionOpened() const;

    bool openConnection();

private:
    void openConnectionIfClosedOrThrow();

public:
    static bool canSendRequestUnicast(NodeState state);

    static std::string toJson(std::shared_ptr<Node> node);

    static std::shared_ptr<Node> fromJson(const nlohmann::json& json);
};

using node_t = std::shared_ptr<Node>;