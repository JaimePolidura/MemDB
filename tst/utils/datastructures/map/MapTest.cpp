#include "gtest/gtest.h"
#include "utils/datastructures/map/Map.h"
#include "string"

struct Value {
    int value;

    Value(int valueCons): value(valueCons) {}
};

Value * insertValueInMap(Map * map, const std::string &key, int value);

TEST(Map, ShouldRemove) {
    Map map{};
    Value * inserted = insertValueInMap(&map, "key1", 1);

    std::optional<MapEntry> removedValueOptional = map.remove("key1");
    const Value * removedValue = (Value *) removedValueOptional.value().value;
    ASSERT_TRUE(removedValueOptional.has_value());
    ASSERT_EQ(removedValue->value, 1);

    ASSERT_FALSE(map.contains("key1"));
    ASSERT_FALSE(map.get("key1").has_value());
}

TEST(Map, ShouldGet) {
    Map map { };

    insertValueInMap(&map, "key1", 1);
    ASSERT_EQ(map.getSize(), 1);

    ASSERT_FALSE(map.get("key2").has_value());
    const std::optional<MapEntry> anOptional = map.get("key1");
    ASSERT_TRUE(anOptional.has_value());
    const Value * valueOptional = (Value *) anOptional.value().value;
    ASSERT_EQ(valueOptional->value, 1);
}

TEST(Map, ShouldPutAndContains) {
    Map map {};

    insertValueInMap(&map, "key1", 1);
    ASSERT_EQ(map.getSize(), 1);
    ASSERT_TRUE(map.contains("key1"));
    ASSERT_FALSE(map.contains("key2"));

    insertValueInMap(&map, "key2", 1123);
    ASSERT_EQ(map.getSize(), 2);
    ASSERT_TRUE(map.contains("key1"));
    ASSERT_TRUE(map.contains("key2"));
}

Value * insertValueInMap(Map * map, const std::string &key, int value){
    Value * value1 = new Value(value);
    map->put(key, (uint8_t *) value1, sizeof(Value));

    return value1;
}