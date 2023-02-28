#include "gtest/gtest.h"
#include "utils/datastructures/map/Map.h"
#include "string"

TEST(Map, ShouldPut) {
    Map<defaultMemDbSize_t> map{64};

    bool placed = map.put(SimpleString<defaultMemDbSize_t>::fromString("nombre"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);
    ASSERT_TRUE(placed);
    ASSERT_EQ(map.getSize(), 1);

    bool placed2 = map.put(SimpleString<defaultMemDbSize_t>::fromString("nombre"), SimpleString<defaultMemDbSize_t>::fromString("pedro"), NOT_IGNORE_TIMESTAMP, 2, 1);
    ASSERT_TRUE(placed2);
    ASSERT_EQ(map.getSize(), 1);
    ASSERT_TRUE(map.contains(SimpleString<defaultMemDbSize_t>::fromString("nombre")));

    const std::optional<MapEntry<defaultMemDbSize_t>> mapEntryNombre = map.get(SimpleString<defaultMemDbSize_t>::fromString("nombre"));
    std::string expectedValue = "pedro";
    ASSERT_TRUE(mapEntryNombre.has_value());
    ASSERT_TRUE(expectedValue.compare(std::string((char *) mapEntryNombre.value().value.data(), mapEntryNombre.value().value.size)) == 0);
}

TEST(Map, ShouldntPut) {
    Map<defaultMemDbSize_t> map{64};

    map.put(SimpleString<defaultMemDbSize_t>::fromString("nombre"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool put = map.put(SimpleString<defaultMemDbSize_t>::fromString("nombre"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(put);
}

TEST(Map, ShoudntGetEmpty) {
    Map<defaultMemDbSize_t> map{64};
    auto result = map.get(SimpleString<defaultMemDbSize_t>::fromString("a"));

    ASSERT_FALSE(result.has_value());
}

TEST(Map, GetAll) {
    Map<defaultMemDbSize_t> map{2};
    map.put(SimpleString<defaultMemDbSize_t>::fromString("A"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<defaultMemDbSize_t>::fromString("B"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<defaultMemDbSize_t>::fromString("C"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<defaultMemDbSize_t>::fromString("D"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);

    auto all = map.all();

    ASSERT_TRUE(all.size() == 4);
}

TEST(Map, ShouldntGetAllEmpty) {
    Map<defaultMemDbSize_t> map{2};
    auto all = map.all();

    ASSERT_TRUE(all.size() == 0);
}

TEST(Map, ShouldRemoveEvenOldTimestamp) {
    Map<defaultMemDbSize_t> map{64};
    map.put(SimpleString<defaultMemDbSize_t>::fromString("A"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<defaultMemDbSize_t>::fromString("A"), IGNORE_TIMESTAMP, 1, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString<defaultMemDbSize_t>::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString<defaultMemDbSize_t>::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveOldTimestamp) {
    Map<defaultMemDbSize_t> map{64};
    map.put(SimpleString<defaultMemDbSize_t>::fromString("A"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<defaultMemDbSize_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(removed);
    ASSERT_EQ(1, map.getSize());
    ASSERT_TRUE(map.contains(SimpleString<defaultMemDbSize_t>::fromString("A")));
    ASSERT_TRUE(map.get(SimpleString<defaultMemDbSize_t>::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveNotFound) {
    Map<defaultMemDbSize_t> map{64};
    bool removed = map.remove(SimpleString<defaultMemDbSize_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_FALSE(removed);
}

TEST(Map, ShouldRemoe) {
    Map<defaultMemDbSize_t> map{64};
    map.put(SimpleString<defaultMemDbSize_t>::fromString("A"), SimpleString<defaultMemDbSize_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<defaultMemDbSize_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString<defaultMemDbSize_t>::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString<defaultMemDbSize_t>::fromString("A")).has_value());
}