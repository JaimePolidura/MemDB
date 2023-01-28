#include "gtest/gtest.h"
#include "utils/datastructures/map/Map.h"
#include "string"

TEST(Map, ShouldPut) {
    Map map{64};

    bool placed = map.put(SimpleString::fromString("nombre"), SimpleString::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);
    ASSERT_TRUE(placed);
    ASSERT_EQ(map.getSize(), 1);

    bool placed2 = map.put(SimpleString::fromString("nombre"), SimpleString::fromString("pedro"), NOT_IGNORE_TIMESTAMP, 2, 1);
    ASSERT_TRUE(placed2);
    ASSERT_EQ(map.getSize(), 1);
    ASSERT_TRUE(map.contains(SimpleString::fromString("nombre")));

    const std::optional<MapEntry> mapEntryNombre = map.get(SimpleString::fromString("nombre"));
    std::string expectedValue = "pedro";
    ASSERT_TRUE(mapEntryNombre.has_value());
    ASSERT_TRUE(expectedValue.compare(std::string((char *) mapEntryNombre.value().value.value, mapEntryNombre.value().value.size)) == 0);
}

TEST(Map, ShouldntPut) {
    Map map{64};

    map.put(SimpleString::fromString("nombre"), SimpleString::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool put = map.put(SimpleString::fromString("nombre"), SimpleString::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(put);
}

TEST(Map, ShoudntGetEmpty) {
    Map map{64};
    auto result = map.get(SimpleString::fromString("a"));

    ASSERT_FALSE(result.has_value());
}

TEST(Map, GetAll) {
    Map map{2};
    map.put(SimpleString::fromString("A"), SimpleString::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString::fromString("B"), SimpleString::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString::fromString("C"), SimpleString::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString::fromString("D"), SimpleString::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);

    auto all = map.all();

    ASSERT_TRUE(all.size() == 4);
}

TEST(Map, ShouldntGetAllEmpty) {
    Map map{2};
    auto all = map.all();

    ASSERT_TRUE(all.size() == 0);
}

TEST(Map, ShouldRemoveEvenOldTimestamp) {
    Map map{64};
    map.put(SimpleString::fromString("A"), SimpleString::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString::fromString("A"), IGNORE_TIMESTAMP, 1, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveOldTimestamp) {
    Map map{64};
    map.put(SimpleString::fromString("A"), SimpleString::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString::fromString("A"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(removed);
    ASSERT_EQ(1, map.getSize());
    ASSERT_TRUE(map.contains(SimpleString::fromString("A")));
    ASSERT_TRUE(map.get(SimpleString::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveNotFound) {
    Map map{64};
    bool removed = map.remove(SimpleString::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_FALSE(removed);
}

TEST(Map, ShouldRemoe) {
    Map map{64};
    map.put(SimpleString::fromString("A"), SimpleString::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString::fromString("A")).has_value());
}