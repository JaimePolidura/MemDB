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
#include "persistence/compression/OplogCompressor.h"

class SyncOplogReceiverIterator : public Iterator<std::vector<uint8_t>> {
private:
    configuration_t configuration;
    clusterNodes_t clusterNodes;
    partitions_t partitions;
    logger_t logger;

    OplogCompressor compressor;

    uint64_t timestampToSync;
    uint32_t selfOplogIdToSync;
    std::function<uint32_t()> nextSyncId;

    uint32_t syncId;
    node_t nodeSender;
    uint64_t nSegmentsRemaining;
    uint32_t nodeOplogIdToSync;

    std::set<memdbNodeId_t> alreadyCheckedNodeIds{};

public:
    SyncOplogReceiverIterator(configuration_t configuration, clusterNodes_t clusterNodes, partitions_t partitions, logger_t logger,
                              uint64_t timestampToSync, uint32_t selfOplogIdToSync, std::function<uint32_t()> nextSyncOplogId);

    bool hasNext() override;

    std::vector<uint8_t> next() override;

    uint64_t totalSize() override;

private:
    void initSyncOplog();

    Response sendNextSegment();

    Request createSyncOplogRequest(memdbNodeId_t nodeIdToSendRequest);
    Request createNextSegmnentRequest();

    std::tuple<uint32_t, uint32_t, uint32_t> createRequestSyncOplogArgs(memdbNodeId_t nodeIdToSendRequest);
    
    std::vector<uint8_t> getOplogFromResponse(Response& response);
};

using syncOplogReceiverIterator_t = std::shared_ptr<SyncOplogReceiverIterator>;