#include "gtest/gtest.h"
#include "utils/DBMap.h"
#include "utils/PolynomialHashCreator.h"
#include "string"
#include "utils/HashCreator.h"

struct Value {
    int value;
};

TEST(DBMap, ShouldPut) {
    HashCreator<std::string> * polynomialHashCreator = dynamic_cast<HashCreator<std::string> *>(new PolynomialHashCreator(31));
    DBMap dbMap { polynomialHashCreator };

    Value value1 {1};
    dbMap.put("key1", (char *) &value1, sizeof(Value));
}