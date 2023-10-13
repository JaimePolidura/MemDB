#include <gtest/gtest.h>

#include "cluster/partitions/Partitions.h"

/**
 * Self = 1
 *
 *          1 (2)
 *  3 (62)          4 (29)
 *          2 (35)
 */

#define HASH_HOLA 5555299136995586291
#define RING_POSITION_KEY_HOLA 51

partitions_t setup();

TEST(Partitions, add){
    partitions_t partitions = setup();
    partitions->add(RingEntry{.nodeId = 5, .ringPosition = 3});

    ASSERT_EQ(partitions->getDistanceClockwise(5), 1);
    ASSERT_EQ(partitions->getDistanceClockwise(4), 2);
    ASSERT_EQ(partitions->getDistanceClockwise(3), 4);

    ASSERT_TRUE(partitions->isNeighbor(5));
    ASSERT_FALSE(partitions->isNeighbor(4));
    ASSERT_TRUE(partitions->isNeighbor(3));

    std::vector<RingEntry> newNeighbors = partitions->getNeighborsClockwise(3);

    ASSERT_TRUE(newNeighbors.size() == 3);
    ASSERT_TRUE(newNeighbors.at(0).nodeId == 5);
    ASSERT_TRUE(newNeighbors.at(1).nodeId == 4);
    ASSERT_TRUE(newNeighbors.at(2).nodeId == 2);
}

TEST(Partitions, deleteByNodeId){
    partitions_t partitions = setup();
    partitions->deleteByNodeId(2);

    ASSERT_EQ(partitions->getDistanceClockwise(4), 1);
    ASSERT_EQ(partitions->getDistanceClockwise(3), 2);

    ASSERT_TRUE(partitions->isNeighbor(3));
    ASSERT_TRUE(partitions->isNeighbor(4));

    std::vector<RingEntry> newNeighbors = partitions->getNeighborsClockwise(3);

    ASSERT_TRUE(newNeighbors.size() == 2);
    ASSERT_TRUE(newNeighbors.at(0).nodeId == 4);
    ASSERT_TRUE(newNeighbors.at(1).nodeId == 3);
}

TEST(Partitions, isNeighbor){
    partitions_t partitions = setup();

    ASSERT_TRUE(partitions->isNeighbor(4));
    ASSERT_FALSE(partitions->isNeighbor(2));
    ASSERT_TRUE(partitions->isNeighbor(3));
}

TEST(Partitions, getDistanceClockwise){
    partitions_t partitions = setup();

    ASSERT_EQ(partitions->getDistanceClockwise(4), 1);
    ASSERT_EQ(partitions->getDistanceClockwise(2), 2);
    ASSERT_EQ(partitions->getDistanceClockwise(3), 3);
}

TEST(Partitions, getDistanceCounterClockwise){
    partitions_t partitions = setup();

    ASSERT_EQ(partitions->getDistanceCounterClockwise(4), 3);
    ASSERT_EQ(partitions->getDistanceCounterClockwise(2), 2);
    ASSERT_EQ(partitions->getDistanceCounterClockwise(3), 1);
}

TEST(Partitions, getDistance){
    partitions_t partitions = setup();

    ASSERT_EQ(partitions->getDistance(4), 1);
    ASSERT_EQ(partitions->getDistance(2), 2);
    ASSERT_EQ(partitions->getDistance(3), -1);
}

TEST(Partitions, canHoldKey) {
    partitions_t partitions = setup();

    ASSERT_FALSE(partitions->canHoldKey(SimpleString<memDbDataLength_t>::fromString("hola")));
}

TEST(Partitions, getDistanceOfKey){
    partitions_t partitions = setup();
    uint32_t distance = partitions->getDistanceOfKey(SimpleString<memDbDataLength_t>::fromString("hola"));

    ASSERT_EQ(distance, 2);
}

TEST(Partitions, getNeighborCounterClockwiseByNodeId){
    partitions_t partitions = setup();

    ASSERT_TRUE(partitions->getNeighborCounterClockwiseByNodeId(2).nodeId == 4);
}

TEST(Partitions, getNeighborsClockwiseByNodeId){
    partitions_t partitions = setup();

    std::vector<RingEntry> neighbors = partitions->getNeighborsClockwiseByNodeId(2);

    ASSERT_TRUE(neighbors.size() == 1);
    ASSERT_TRUE(neighbors[0].nodeId == 3);
}

TEST(Partitions, getNeighborsClockwise){
    partitions_t partitions = setup();

    std::vector<RingEntry> neighbors = partitions->getNeighborsClockwise();

    ASSERT_TRUE(neighbors.size() == 1);
    ASSERT_TRUE(neighbors[0].nodeId == 4);
}

TEST(Partitions, isClockwiseNeighbor){
    partitions_t partitions = setup();

    ASSERT_TRUE(partitions->isClockwiseNeighbor(4));
    ASSERT_FALSE(partitions->isClockwiseNeighbor(2));
    ASSERT_FALSE(partitions->isClockwiseNeighbor(3));
}

TEST(Partitions, getRingPositionByKey){
    uint32_t ringPosition = setup()
            ->getRingPositionByKey(SimpleString<memDbDataLength_t>::fromString("test"));

    ASSERT_EQ(ringPosition, RING_POSITION_KEY_HOLA);
}

partitions_t setup() {
    std::map<std::string, std::string> configValues = {{ConfigurationKeys::NODE_ID, "1"}};
    configuration_t configuration = std::make_shared<Configuration>(configValues);

    int maxSize = 64;
    int nodesPerPartition = 2;
    std::vector<RingEntry> entries = {
            RingEntry{.nodeId = 1, .ringPosition = 2},
            RingEntry{.nodeId = 4, .ringPosition = 29},
            RingEntry{.nodeId = 2, .ringPosition = 35},
            RingEntry{.nodeId = 3, .ringPosition = 62}
    };

    return std::make_shared<Partitions>(entries, nodesPerPartition, maxSize, configuration);
}