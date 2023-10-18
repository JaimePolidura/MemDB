#include "SyncOplogReceiverIterator.h"

SyncOplogReceiverIterator::SyncOplogReceiverIterator(configuration_t configuration, clusterNodes_t clusterNodes, partitions_t partitions,
    uint64_t timestampToSync, uint32_t oplogIdToSync, std::function<uint32_t()> nextSyncOplogId): oplogIdToSync(oplogIdToSync),
    configuration(configuration), partitions(partitions), nextSyncOplogId(nextSyncOplogId), clusterNodes(clusterNodes), timestampToSync(timestampToSync) {
    this->initSyncOplog();
}

bool SyncOplogReceiverIterator::hasNext() {
    return !this->eofReached || this->nSegmentsRemaining > 0;
}

std::vector<uint8_t> SyncOplogReceiverIterator::next() {
    Response nextResponse = this->sendNextSegment();

    if(!nextResponse.isSuccessful){
        this->eofReached = true;
        return std::vector<uint8_t>{};
    }

    this->timestampToSync = nextResponse.timestamp;
    this->nSegmentsRemaining--;

    return nextResponse.responseValue.toVector();
}

Response SyncOplogReceiverIterator::sendNextSegment() {
    std::optional<Response> responseOptional{};

    do {
        responseOptional = this->nodeSender->sendRequest(createNextSegmnentRequest(), true);

        if(!responseOptional.has_value()){
            this->initSyncOplog();
        }
    }while(responseOptional.has_value());

    return responseOptional.value();
}

uint64_t SyncOplogReceiverIterator::totalSize() {
    return 0; //Unused TODO revise
}

void SyncOplogReceiverIterator::initSyncOplog() {
    while(true) {
        node_t node = Utils::getOptionalOrThrow(this->clusterNodes->getRandomNode(this->alreadyCheckedNodeIds, {.partitionId = (int) this->oplogIdToSync}),
                                                "No node available to select for syncing oplog");

        Request request = this->createSyncOplogRequest(node->nodeId);

        std::optional<Response> responseOptional = node->sendRequest(request, true);

        if(responseOptional.has_value()){
            this->nSegmentsRemaining = responseOptional.value().responseValue.to<uint32_t>();
            this->syncOplogId = responseOptional->requestNumber;
            this->nodeSender = node;
        } else {
            this->eofReached = true;
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
            ->requestNumber(this->nextSyncOplogId()) // Used as multi-response Id
            ->build();
}

Request SyncOplogReceiverIterator::createNextSegmnentRequest() {
    auto [part1, part2, nodeOplogId] = this->createRequestSyncOplogArgs(this->nodeSender->nodeId);

    return RequestBuilder::builder()
            .authKey(this->configuration->get(ConfigurationKeys::AUTH_NODE_KEY))
            ->operatorNumber(OperatorNumbers::NEXT_SYNC_OPLOG_SEGMENT)
            ->requestNumber(this->oplogIdToSync)
            ->args({
                SimpleString<memDbDataLength_t>::fromNumber(part1),
                SimpleString<memDbDataLength_t>::fromNumber(part2),
                SimpleString<memDbDataLength_t>::fromNumber(this->oplogIdToSync)
            })
            ->build();
}

std::tuple<uint32_t, uint32_t, uint32_t> SyncOplogReceiverIterator::createRequestSyncOplogArgs(memdbNodeId_t nodeIdToSendRequest) {
    uint32_t part1 = this->timestampToSync >> 32;
    uint32_t part2 = this->timestampToSync & 0xFFFFFFFF;
    uint32_t nodeOplogId = this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS) ?
                           (this->partitions->isClockwiseNeighbor(this->nodeSender->nodeId) ?
                            this->oplogIdToSync + this->partitions->getDistanceClockwise(nodeIdToSendRequest) :
                            this->oplogIdToSync - this->partitions->getDistanceCounterClockwise(nodeIdToSendRequest)) : 0;

    return std::make_tuple(part1, part2, nodeOplogId);
}