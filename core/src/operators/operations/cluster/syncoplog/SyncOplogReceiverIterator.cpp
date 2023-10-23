#include "SyncOplogReceiverIterator.h"

SyncOplogReceiverIterator::SyncOplogReceiverIterator(configuration_t configuration, clusterNodes_t clusterNodes, partitions_t partitions, logger_t logger,
        uint64_t timestampToSync, uint32_t selfOplogIdToSync, std::function<uint32_t()> nextSyncOplogId): selfOplogIdToSync(selfOplogIdToSync),
        configuration(configuration), partitions(partitions), nextSyncId(nextSyncOplogId), clusterNodes(clusterNodes), logger(logger), timestampToSync(timestampToSync) {
    this->initSyncOplog();
}

bool SyncOplogReceiverIterator::hasNext() {
    return this->nSegmentsRemaining > 0;
}

std::vector<uint8_t> SyncOplogReceiverIterator::next() {
    Response nextResponse = this->sendNextSegment();

    if(!nextResponse.isSuccessful){
        this->nSegmentsRemaining = 0;
        return std::vector<uint8_t>{};
    }

    this->timestampToSync = nextResponse.timestamp;
    this->nSegmentsRemaining--;

    this->logger->debugInfo("Received new segment from node {0} of {1} bytes at timestamp {2} Segments remaining: {3}",
                            this->nodeSender->nodeId, nextResponse.responseValue.size, this->timestampToSync, this->nSegmentsRemaining);

    return nextResponse.responseValue.toVector();
}

Response SyncOplogReceiverIterator::sendNextSegment() {
    std::optional<Response> responseOptional{};

    do {
        responseOptional = this->nodeSender->sendRequest(createNextSegmnentRequest());

        if(!responseOptional.has_value()){
            this->logger->debugInfo("Restarting SYNC_OPLOG at timestamp {0}", this->timestampToSync);
            this->initSyncOplog();
        }
    }while(!responseOptional.has_value());

    return responseOptional.value();
}

uint64_t SyncOplogReceiverIterator::totalSize() {
    return this->nSegmentsRemaining;
}

void SyncOplogReceiverIterator::initSyncOplog() {
    while(true){
        node_t node = Utils::getOptionalOrThrow(this->clusterNodes->getRandomNode(this->alreadyCheckedNodeIds, {.partitionId = (int) this->selfOplogIdToSync}),
                                                "No node available to select for syncing oplog");

        Request request = this->createSyncOplogRequest(node->nodeId);

        std::optional<Response> responseOptional = node->sendRequest(request);

        if(responseOptional.has_value()){
            this->nSegmentsRemaining = responseOptional.value().responseValue.to<uint64_t>();
            this->syncId = responseOptional->requestNumber;
            this->nodeSender = node;

            this->logger->debugInfo("Initiated SYNC_OPLOG with node {0} syncId {1} total n segments to receive {2} at timestamp {3}",
                                    node->nodeId, this->syncId, this->nSegmentsRemaining, this->timestampToSync);

            return;
        }
    }
}

Request SyncOplogReceiverIterator::createSyncOplogRequest(memdbNodeId_t nodeIdToSendRequest) {
    auto [part1, part2, nodeOplogId] = this->createRequestSyncOplogArgs(nodeIdToSendRequest);

    return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->args({
                SimpleString<memDbDataLength_t>::fromNumber(part1),
                SimpleString<memDbDataLength_t>::fromNumber(part2),
                SimpleString<memDbDataLength_t>::fromNumber(nodeOplogId)
            })
            ->selfNode(this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID))
            ->operatorNumber(OperatorNumbers::SYNC_OPLOG)
            ->requestNumber(this->nextSyncId())
            ->build();
}

Request SyncOplogReceiverIterator::createNextSegmnentRequest() {
    auto [part1, part2, nodeOplogId] = this->createRequestSyncOplogArgs(this->nodeSender->nodeId);

    return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->operatorNumber(OperatorNumbers::NEXT_SYNC_OPLOG_SEGMENT)
            ->requestNumber(this->syncId)
            ->args({
                SimpleString<memDbDataLength_t>::fromNumber(part1),
                SimpleString<memDbDataLength_t>::fromNumber(part2),
                SimpleString<memDbDataLength_t>::fromNumber(nodeOplogId)
            })
            ->build();
}

std::tuple<uint32_t, uint32_t, uint32_t> SyncOplogReceiverIterator::createRequestSyncOplogArgs(memdbNodeId_t nodeIdToSendRequest) {
    uint32_t part1 = this->timestampToSync >> 32;
    uint32_t part2 = this->timestampToSync & 0xFFFFFFFF;
    uint32_t nodeOplogId = this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS) ?
                           (this->partitions->isClockwiseNeighbor(nodeIdToSendRequest) ?
                            this->selfOplogIdToSync + this->partitions->getDistanceClockwise(nodeIdToSendRequest) :
                            this->selfOplogIdToSync - this->partitions->getDistanceCounterClockwise(nodeIdToSendRequest))
                            : 0;

    return std::make_tuple(part1, part2, nodeOplogId);
}