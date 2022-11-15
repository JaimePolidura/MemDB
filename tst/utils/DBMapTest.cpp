#include "gtest/gtest.h"
#include "utils/DBMap.h"
#include "utils/PolynomialHashCreator.h"
#include "string"
#include "utils/HashCreator.h"

struct Value {
    int value;
};

void insertValueInMap(DBMap * dbmap, const std::string &key, const int value);
bool stringCompare(char * a, char * b);

TEST(DBMap, ShouldGet) {
    HashCreator<std::string> * polynomialHashCreator = dynamic_cast<HashCreator<std::string> *>(new PolynomialHashCreator(31));
    DBMap dbMap { polynomialHashCreator };

    insertValueInMap(&dbMap, "key1", 1);
    ASSERT_EQ(dbMap.getSize(), 1);
    ASSERT_TRUE(dbMap.get("key1").has_value());
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

void insertValueInMap(DBMap * dbmap, const std::string &key, const int value){
    Value value1 {value};
    dbmap->put(key, (char *) &value1, sizeof(Value));
}