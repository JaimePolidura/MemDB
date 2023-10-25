#include "gtest/gtest.h"
#include "utils/datastructures/map/Map.h"
#include "string"

TEST(Map, ShouldPut) {
    Map<memDbDataLength_t> map{64};

    bool placed = map.put(SimpleString<memDbDataLength_t>::fromString("nombre"), SimpleString<memDbDataLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);
    ASSERT_TRUE(placed);
    ASSERT_EQ(map.getSize(), 1);

    bool placed2 = map.put(SimpleString<memDbDataLength_t>::fromString("nombre"), SimpleString<memDbDataLength_t>::fromString("pedro"), NOT_IGNORE_TIMESTAMP, 2, 1);
    ASSERT_TRUE(placed2);
    ASSERT_EQ(map.getSize(), 1);
    ASSERT_TRUE(map.contains(SimpleString<memDbDataLength_t>::fromString("nombre")));

    const std::optional<MapEntry<memDbDataLength_t>> mapEntryNombre = map.get(SimpleString<memDbDataLength_t>::fromString("nombre"));
    std::string expectedValue = "pedro";
    ASSERT_TRUE(mapEntryNombre.has_value());
    ASSERT_TRUE(expectedValue.compare(
            std::string((char *) mapEntryNombre._value()._value.data(), mapEntryNombre._value()._value.totalSize)) == 0);
}

TEST(Map, ShouldntPut) {
    Map<memDbDataLength_t> map{64};

    map.put(SimpleString<memDbDataLength_t>::fromString("nombre"), SimpleString<memDbDataLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool put = map.put(SimpleString<memDbDataLength_t>::fromString("nombre"), SimpleString<memDbDataLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(put);
}

TEST(Map, ShoudntGetEmpty) {
    Map<memDbDataLength_t> map{64};
    auto result = map.get(SimpleString<memDbDataLength_t>::fromString("a"));

    ASSERT_FALSE(result.has_value());
}

TEST(Map, GetAll) {
    Map<memDbDataLength_t> map{2};
    map.put(SimpleString<memDbDataLength_t>::fromString("A"), SimpleString<memDbDataLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<memDbDataLength_t>::fromString("B"), SimpleString<memDbDataLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<memDbDataLength_t>::fromString("C"), SimpleString<memDbDataLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    map.put(SimpleString<memDbDataLength_t>::fromString("D"), SimpleString<memDbDataLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);

    auto all = map.all();

    ASSERT_TRUE(all.totalSize() == 4);
}

TEST(Map, ShouldntGetAllEmpty) {
    Map<memDbDataLength_t> map{2};
    auto all = map.all();

    ASSERT_TRUE(all.totalSize() == 0);
}

TEST(Map, ShouldRemoveEvenOldTimestamp) {
    Map<memDbDataLength_t> map{64};
    map.put(SimpleString<memDbDataLength_t>::fromString("A"), SimpleString<memDbDataLength_t>::fromString("jaime"), IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<memDbDataLength_t>::fromString("A"), IGNORE_TIMESTAMP, 1, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString<memDbDataLength_t>::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString<memDbDataLength_t>::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveOldTimestamp) {
    Map<memDbDataLength_t> map{64};
    map.put(SimpleString<memDbDataLength_t>::fromString("A"), SimpleString<memDbDataLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<memDbDataLength_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 1, 1);

    ASSERT_FALSE(removed);
    ASSERT_EQ(1, map.getSize());
    ASSERT_TRUE(map.contains(SimpleString<memDbDataLength_t>::fromString("A")));
    ASSERT_TRUE(map.get(SimpleString<memDbDataLength_t>::fromString("A")).has_value());
}

TEST(Map, ShouldntRemoveNotFound) {
    Map<memDbDataLength_t> map{64};
    bool removed = map.remove(SimpleString<memDbDataLength_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_FALSE(removed);
}

TEST(Map, ShouldRemoe) {
    Map<memDbDataLength_t> map{64};
    map.put(SimpleString<memDbDataLength_t>::fromString("A"), SimpleString<memDbDataLength_t>::fromString("jaime"), NOT_IGNORE_TIMESTAMP, 2, 1);
    bool removed = map.remove(SimpleString<memDbDataLength_t>::fromString("A"), NOT_IGNORE_TIMESTAMP, 3, 1);

    ASSERT_TRUE(removed);
    ASSERT_EQ(0, map.getSize());
    ASSERT_FALSE(map.contains(SimpleString<memDbDataLength_t>::fromString("A")));
    ASSERT_FALSE(map.get(SimpleString<memDbDataLength_t>::fromString("A")).has_value());
}