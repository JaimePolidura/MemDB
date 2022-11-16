#include "gtest/gtest.h"
#include "utils/crypto/PolynomialHashCreator.h"

TEST(PolynomialHashCreator, PolynomialHashCreatorReturnsSameValueForSameArgs){
    PolynomialHashCreator polynomialHashCreator = {31};

    ASSERT_EQ(polynomialHashCreator.create("hola"), polynomialHashCreator.create("hola"));
    ASSERT_EQ(polynomialHashCreator.create("adios"), polynomialHashCreator.create("adios"));
}

TEST(PolynomialHashCreator, PolynomialHashCreatorReturnsDifferentValuesForDifferentArgs) {
    PolynomialHashCreator polynomialHashCreator = {31};

    ASSERT_TRUE(polynomialHashCreator.create("hola") != polynomialHashCreator.create("adios"));
    ASSERT_TRUE(polynomialHashCreator.create("hola") != polynomialHashCreator.create("holaa"));
}