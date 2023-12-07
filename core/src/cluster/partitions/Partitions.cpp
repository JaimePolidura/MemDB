#include "cluster/partitions/Partitions.h"

Partitions::Partitions(configuration_t configuration): configuration(configuration) {}

Partitions::Partitions(const std::vector<RingEntry>& allRingEntries, uint32_t nodesPerPartition, uint32_t maxSize, configuration_t configuration):
        nodesPerPartition(nodesPerPartition), maxSize(maxSize), ringEntries(RingEntries::fromEntries(allRingEntries)), configuration(configuration) {
    this->selfEntry = this->ringEntries.getByNodeId(configuration->get<memdbNodeId_t>(ConfigurationKeys::NODE_ID));
}

uint32_t Partitions::getMaxSize() {
    return this->maxSize;
}

std::vector<RingEntry> Partitions::getAll() {
    return this->ringEntries.getAll();
}

uint32_t Partitions::getOplogIdOfOtherNodeBySelfOplogId(memdbNodeId_t otherNodeId, uint32_t selfOplogId) {
    return this->configuration->getBoolean(ConfigurationKeys::USE_PARTITIONS) ?
           (this->isClockwiseNeighbor(otherNodeId) ?
            selfOplogId + this->getDistanceClockwise(otherNodeId) :
            selfOplogId - this->getDistanceCounterClockwise(otherNodeId))
        : 0;
}

uint32_t Partitions::getRingPositionByKey(SimpleString<memDbDataLength_t> key) {
    return HashCalculator::calculateMD5(key.toString()) % this->maxSize;
}

// self --> (clockwise) nodeB
bool Partitions::isClockwiseNeighbor(memdbNodeId_t nodeB) {
    return this->ringEntries.getDistanceClockwise(this->selfEntry.nodeId, nodeB) < this->nodesPerPartition;
}

std::vector<RingEntry> Partitions::getNeighborsClockwise(int numberNeighbors) {
    return this->ringEntries.getNeighborsClockwise(this->selfEntry.nodeId, numberNeighbors == -1 ? this->nodesPerPartition - 1 : numberNeighbors);
}

std::vector<RingEntry> Partitions::getNeighborsClockwiseByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getNeighborsClockwise(nodeId, this->nodesPerPartition - 1);
}

std::vector<RingEntry> Partitions::getNeighborsCounterClockwiseByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getNeighborsCounterClockwise(nodeId, this->nodesPerPartition - 1);
}

std::vector<RingEntry> Partitions::getNeighbors() {
    std::vector<RingEntry> neighboursCounterClockwise = this->getNeighborsCounterClockwiseByNodeId(this->selfEntry.nodeId);
    std::vector<RingEntry> neighboursClockWise = this->getNeighborsClockwise();

    return Utils::concat(neighboursCounterClockwise, neighboursClockWise);
}

std::optional<RingEntry> Partitions::getNeighborCounterClockwiseByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getNeighborCounterClockwise(nodeId);
}

std::optional<RingEntry> Partitions::getNeighborClockwiseByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getNeighborClockwise(nodeId);
}

uint32_t Partitions::getRingPositionByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getByNodeId(nodeId).ringPosition;
}

uint32_t Partitions::getDistanceOfKey(SimpleString<memDbDataLength_t> key) {
    uint32_t ringPosition = this->getRingPositionByKey(key);
    uint32_t nodeThatWouldHoldThatKey = this->ringEntries.getRingEntryBelongsToPosition(ringPosition).nodeId;

    return std::abs(this->ringEntries.getDistance(this->selfEntry.nodeId, nodeThatWouldHoldThatKey));
}

bool Partitions::canHoldKey(SimpleString<memDbDataLength_t> key) {
    return this->getDistanceOfKey(key) < this->nodesPerPartition;
}

int Partitions::getDistance(memdbNodeId_t nodeB) {
    return this->ringEntries.getDistance(this->selfEntry.nodeId, nodeB);
}

// self <-- (memdb_thread_pool_counter clockwise) nodeB
uint32_t Partitions::getDistanceCounterClockwise(memdbNodeId_t nodeB) {
    return this->ringEntries.getDistanceCounterClockwise(this->selfEntry.nodeId, nodeB);
}

// self --> (clockwise) nodeB
uint32_t Partitions::getDistanceClockwise(memdbNodeId_t nodeB) {
    return this->ringEntries.getDistanceClockwise(this->selfEntry.nodeId, nodeB);
}

bool Partitions::isNeighbor(memdbNodeId_t otherNode) {
    return std::abs(this->ringEntries.getDistance(this->selfEntry.nodeId, otherNode)) < this->nodesPerPartition;
}

void Partitions::deleteByNodeId(memdbNodeId_t nodeId) {
    this->ringEntries.deleteByNodeId(nodeId);
}

void Partitions::add(RingEntry ringEntry){
    this->ringEntries.add(ringEntry);
}

RingEntry Partitions::addByNode(node_t node) {
    uint32_t ringPosition = static_cast<uint32_t>(HashCalculator::calculateMD5(std::to_string(node->nodeId)) % this->maxSize);

    RingEntry ringEntry = RingEntry{
        .nodeId = node->nodeId,
        .ringPosition = ringPosition
    };

    this->ringEntries.add(ringEntry);

    return ringEntry;
}

uint32_t Partitions::getNodesPerPartition() {
    return this->nodesPerPartition;
}

RingEntry Partitions::getByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getByNodeId(nodeId);
}

RingEntry Partitions::getSelfEntry() {
    return this->selfEntry;
}