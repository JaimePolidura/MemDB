#include "gtest/gtest.h"
#include "utils/datastructures/map/DBMap.h"
#include "utils/crypto/PolynomialHashCreator.h"
#include "string"
#include "utils/crypto/HashCreator.h"

struct Value {
    int value;

    Value(int valueCons): value(valueCons) {}
};

Value * insertValueInMap(DBMap * dbmap, const std::string &key, int value);

TEST(DBMap, ShouldRemove) {
    HashCreator<std::string> * polynomialHashCreator = dynamic_cast<HashCreator<std::string> *>(new PolynomialHashCreator(31));
    DBMap dbMap { polynomialHashCreator };
    Value * inserted = insertValueInMap(&dbMap, "key1", 1);

    std::optional<MapEntry> removedValueOptional = dbMap.remove("key1");
    const Value * removedValue = (Value *) removedValueOptional.value().value;
    ASSERT_TRUE(removedValueOptional.has_value());
    ASSERT_EQ(removedValue->value, 1);

    ASSERT_FALSE(dbMap.contains("key1"));
    ASSERT_FALSE(dbMap.get("key1").has_value());
}

TEST(DBMap, ShouldGet) {
    HashCreator<std::string> * polynomialHashCreator = dynamic_cast<HashCreator<std::string> *>(new PolynomialHashCreator(31));
    DBMap dbMap { polynomialHashCreator };

    insertValueInMap(&dbMap, "key1", 1);
    ASSERT_EQ(dbMap.getSize(), 1);

    ASSERT_FALSE(dbMap.get("key2").has_value());
    const std::optional<MapEntry> anOptional = dbMap.get("key1");
    ASSERT_TRUE(anOptional.has_value());
    const Value * valueOptional = (Value *) anOptional.value().value;
    ASSERT_EQ(valueOptional->value, 1);
}

TEST(DBMap, ShouldPutAndContains) {
    HashCreator<std::string> * polynomialHashCreator = dynamic_cast<HashCreator<std::string> *>(new PolynomialHashCreator(31));
    DBMap dbMap { polynomialHashCreator };

    insertValueInMap(&dbMap, "key1", 1);
    ASSERT_EQ(dbMap.getSize(), 1);
    ASSERT_TRUE(dbMap.contains("key1"));
    ASSERT_FALSE(dbMap.contains("key2"));

    insertValueInMap(&dbMap, "key2", 1123);
    ASSERT_EQ(dbMap.getSize(), 2);
    ASSERT_TRUE(dbMap.contains("key1"));
    ASSERT_TRUE(dbMap.contains("key2"));
}

Value * insertValueInMap(DBMap * dbmap, const std::string &key, int value){
    Value * value1 = new Value(value);
    dbmap->put(key, (char *) value1, sizeof(Value));

    return value1;
}