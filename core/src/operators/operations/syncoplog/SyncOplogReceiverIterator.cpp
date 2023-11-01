#include "SyncOplogReceiverIterator.h"

SyncOplogReceiverIterator::SyncOplogReceiverIterator(configuration_t configuration, clusterNodes_t clusterNodes, partitions_t partitions, logger_t loggerCons,
        uint64_t timestampToSync, uint32_t selfOplogIdToSync, std::function<uint32_t()> nextSyncOplogId): 
        selfOplogIdToSync(selfOplogIdToSync),
        configuration(configuration),
        partitions(partitions), 
        nextSyncId(nextSyncOplogId), 
        clusterNodes(clusterNodes), 
        logger(loggerCons), 
        timestampToSync(timestampToSync) {
    this->initSyncOplog();
}

bool SyncOplogReceiverIterator::hasNext() {
    return this->nSegmentsRemaining > 0;
}

std::result<std::vector<uint8_t>> SyncOplogReceiverIterator::next() {
    Response nextResponse = this->sendNextSegment();

    if(!nextResponse.isSuccessful && nextResponse.hasErrorCode(ErrorCode::SYNC_OP_LOG_EOF)){
        this->nSegmentsRemaining = 0;
        return std::ok(std::vector<uint8_t>{});
    }
    if(!nextResponse.isSuccessful && nextResponse.hasErrorCode(ErrorCode::UNFIXABLE_CORRUPTED_OPLOG_SEGMENT)){
        return std::error(std::vector<uint8_t>{});
    }

    this->timestampToSync = nextResponse.timestamp;
    this->nSegmentsRemaining--;

    this->logger->debugInfo("Received new segment from node {0} of {1} bytes at timestamp {2} Segments remaining: {3}",
                            this->nodeSender->nodeId, nextResponse.responseValue.size, this->timestampToSync, this->nSegmentsRemaining);

    return this->getOplogFromResponse(nextResponse);
}

Response SyncOplogReceiverIterator::sendNextSegment() {
    std::result<Response> responseResult{};

    do {
        responseResult = this->nodeSender->sendRequest(createNextSegmnentRequest());

        if(responseResult.has_error()){
            this->logger->debugInfo("Restarting SYNC_OPLOG at timestamp {0}", this->timestampToSync);
            this->initSyncOplog();
        }
    }while(responseResult.has_error());

    return responseResult.get();
}

uint64_t SyncOplogReceiverIterator::totalSize() {
    return this->nSegmentsRemaining;
}

std::result<std::vector<uint8_t>> SyncOplogReceiverIterator::getOplogFromResponse(Response& response) {
    auto oplog = response.getResponseValueAtOffset(4, response.responseValue.size - 4).toVector();
    auto originalSize = response.getResponseValueAtOffset(0, 4).to<uint32_t>();

    return std::ok(this->compressor.uncompressBytes(oplog, originalSize).get_or_throw_with([](const int errorCode) {
        return "Unable to uncompress oplog in SyncOplogReceiverIterator::getOplogFromResponse with error code: " + errorCode;
    }));
}

void SyncOplogReceiverIterator::initSyncOplog() {
    while(true){
        node_t node = Utils::getOptionalOrThrow(this->clusterNodes->getRandomNode(this->alreadyCheckedNodeIds, {.partitionId = (int) this->selfOplogIdToSync}),
                                                "No node available to select for syncing oplog");

        Request request = this->createSyncOplogRequest(node->nodeId);

        std::result<Response> responseResult = node->sendRequest(request);

        if(responseResult.is_success()){
            this->nSegmentsRemaining = responseResult->responseValue.to<uint64_t>();
            this->syncId = responseResult->requestNumber;
            this->nodeSender = node;

            this->logger->debugInfo("Initiated SYNC_OPLOG with node {0} syncId {1} total n segments to receive {2} at timestamp {3}",
                                    node->nodeId, this->syncId, this->nSegmentsRemaining, this->timestampToSync);

            return;
        }
    }
}

Request SyncOplogReceiverIterator::createSyncOplogRequest(memdbNodeId_t nodeIdToSendRequest) {
    auto nodeOplogId = this->partitions->getOplogIdOfOtherNodeBySelfOplogId(nodeIdToSendRequest, this->selfOplogIdToSync);

    return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->addDoubleArg(this->timestampToSync)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(nodeOplogId))
            ->selfNode(this->configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID))
            ->operatorNumber(OperatorNumbers::SYNC_OPLOG)
            ->requestNumber(this->nextSyncId())
            ->build();
}

Request SyncOplogReceiverIterator::createNextSegmnentRequest() {
    auto nodeOplogId = this->partitions->getOplogIdOfOtherNodeBySelfOplogId(this->nodeSender->nodeId, this->selfOplogIdToSync);

    return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->operatorNumber(OperatorNumbers::NEXT_SYNC_OPLOG_SEGMENT)
            ->requestNumber(this->syncId)
            ->addDoubleArg(this->timestampToSync)
            ->addArg(SimpleString<memDbDataLength_t>::fromNumber(nodeOplogId))
            ->build();
}