#include "cluster/partitions/Partitions.h"

Partitions::Partitions(const std::vector<RingEntry>& allRingEntries, uint32_t nodesPerPartition, uint32_t maxSize, configuration_t configuration):
        nodesPerPartition(nodesPerPartition), maxSize(maxSize), ringEntries(RingEntries::fromEntries(allRingEntries)), configuration(configuration) {
    this->selfEntry = this->ringEntries.getByNodeId(configuration->get<memdbNodeId_t>(ConfigurationKeys::MEMDB_CORE_NODE_ID));
}

uint32_t Partitions::getRingPositionByKey(SimpleString<memDbDataLength_t> key) const {
    return HashCalculator::calculate(key.toString()) % this->maxSize;
}

// self --> (clockwise) nodeB
bool Partitions::isClockwiseNeighbor(memdbNodeId_t nodeB) {
    return this->ringEntries.getDistanceClockwise(this->selfEntry.nodeId, nodeB) < this->nodesPerPartition;
}

std::vector<RingEntry> Partitions::getNeighborsClockwise(int numberNeighbors) {
    return this->ringEntries.getNeighborsClockwise(this->selfEntry.nodeId,numberNeighbors == -1 ? this->nodesPerPartition - 1 : numberNeighbors);
}

std::vector<RingEntry> Partitions::getNeighborsClockwiseByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getNeighborsClockwise(nodeId, this->nodesPerPartition - 1);
}

RingEntry Partitions::getNeighborCounterClockwiseByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getNeighborCounterClockwise(nodeId);
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

// self <-- (counter clockwise) nodeB
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

uint32_t Partitions::getNodesPerPartition() const {
    return this->nodesPerPartition;
}

RingEntry Partitions::getByNodeId(memdbNodeId_t nodeId) {
    return this->ringEntries.getByNodeId(nodeId);
}

RingEntry Partitions::getSelfEntry() {
    return this->selfEntry;
}