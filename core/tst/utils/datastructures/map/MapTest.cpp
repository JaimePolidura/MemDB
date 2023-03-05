#include "gtest/gtest.h"
#include "utils/datastructures/map/Map.h"
#include "string"

TEST(Map, ShouldPut) {
    Map<defaultMemDbLength_t> map{64};

    bool placed = map.put(SimpleString<defaultMemDbLength_t>::fromString("nombre"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);
    ASSERT_TRUE(placed);
    ASSERT_EQ(map.getSize(), 1);

    bool placed2 = map.put(SimpleString<defaultMemDbLength_t>::fromString("nombre"), SimpleString<defaultMemDbLength_t>::fromString("pedro"), NOT_IGNORE_TIMESTAMP, 2, 1);
    ASSERT_TRUE(placed2);
    ASSERT_EQ(map.getSize(), 1);
    ASSERT_TRUE(map.contains(SimpleString<defaultMemDbLength_t>::fromString("nombre")));

    const std::optional<MapEntry<defaultMemDbLength_t>> mapEntryNombre = map.get(SimpleString<defaultMemDbLength_t>::fromString("nombre"));
    std::string expectedValue = "pedro";
    ASSERT_TRUE(mapEntryNombre.has_value());
    ASSERT_TRUE(expectedValue.compare(std::string((char *) mapEntryNombre.value().value.data(), mapEntryNombre.value().value.size)) == 0);
}

TEST(Map, ShouldntPut) {
    Map<defaultMemDbLength_t> map{64};

    map.put(SimpleString<defaultMemDbLength_t>::fromString("nombre"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool put = map.put(SimpleString<defaultMemDbLength_t>::fromString("nombre"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(put);
}

TEST(Map, ShoudntGetEmpty) {
    Map<defaultMemDbLength_t> map{64};
    auto result = map.get(SimpleString<defaultMemDbLength_t>::fromString("a"));

    ASSERT_FALSE(result.has_value());
}

TEST(Map, GetAll) {
    Map<defaultMemDbLength_t> map{2};
    map.put(SimpleString<defaultMemDbLength_t>::fromString("A"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<defaultMemDbLength_t>::fromString("B"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<defaultMemDbLength_t>::fromString("C"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<defaultMemDbLength_t>::fromString("D"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);

    auto all = map.all();

    ASSERT_TRUE(all.size() == 4);
}

TEST(Map, ShouldntGetAllEmpty) {
    Map<defaultMemDbLength_t> map{2};
    auto all = map.all();

    ASSERT_TRUE(all.size() == 0);
}

TEST(Map, ShouldRemoveEvenOldTimestamp) {
    Map<defaultMemDbLength_t> map{64};
    map.put(SimpleString<defaultMemDbLength_t>::fromString("A"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<defaultMemDbLength_t>::fromString("A"), IGNORE_TIMESTAMP, 1, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString<defaultMemDbLength_t>::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString<defaultMemDbLength_t>::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveOldTimestamp) {
    Map<defaultMemDbLength_t> map{64};
    map.put(SimpleString<defaultMemDbLength_t>::fromString("A"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<defaultMemDbLength_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(removed);
    ASSERT_EQ(1, map.getSize());
    ASSERT_TRUE(map.contains(SimpleString<defaultMemDbLength_t>::fromString("A")));
    ASSERT_TRUE(map.get(SimpleString<defaultMemDbLength_t>::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveNotFound) {
    Map<defaultMemDbLength_t> map{64};
    bool removed = map.remove(SimpleString<defaultMemDbLength_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_FALSE(removed);
}

TEST(Map, ShouldRemoe) {
    Map<defaultMemDbLength_t> map{64};
    map.put(SimpleString<defaultMemDbLength_t>::fromString("A"), SimpleString<defaultMemDbLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<defaultMemDbLength_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString<defaultMemDbLength_t>::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString<defaultMemDbLength_t>::fromString("A")).has_value());
}