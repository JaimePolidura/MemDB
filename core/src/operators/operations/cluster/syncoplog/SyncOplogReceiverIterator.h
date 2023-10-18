#pragma once

#include "shared.h"
#include "utils/strings/SimpleString.h"
#include "utils/Iterator.h"
#include "messages/response/Response.h"
#include "messages/request/RequestBuilder.h"
#include "cluster/othernodes/ClusterNodes.h"
#include "config/Configuration.h"
#include "cluster/partitions/Partitions.h"
#include "operators/OperatorNumbers.h"

class SyncOplogReceiverIterator : public Iterator<std::vector<uint8_t>> {
private:
    configuration_t configuration;
    clusterNodes_t clusterNodes;
    partitions_t partitions;

    uint64_t timestampToSync;
    uint32_t oplogIdToSync;
    std::function<uint32_t()> nextSyncOplogId;

    uint32_t syncOplogId;
    node_t nodeSender;
    uint32_t nSegmentsRemaining;
    bool eofReached{false};

    std::set<memdbNodeId_t> alreadyCheckedNodeIds{};

public:
    SyncOplogReceiverIterator(configuration_t configuration, clusterNodes_t clusterNodes, partitions_t partitions,
                              uint64_t timestampToSync, uint32_t oplogIdToSync, std::function<uint32_t()> nextSyncOplogId);

    bool hasNext() override;

    std::vector<uint8_t> next() override;

    uint64_t totalSize() override;

private:
    void initSyncOplog();

    Response sendNextSegment();

    Request createSyncOplogRequest(memdbNodeId_t nodeIdToSendRequest);
    Request createNextSegmnentRequest();

    std::tuple<uint32_t, uint32_t, uint32_t> createRequestSyncOplogArgs(memdbNodeId_t nodeIdToSendRequest);
};

using syncOplogReceiverIterator_t = std::shared_ptr<SyncOplogReceiverIterator>;