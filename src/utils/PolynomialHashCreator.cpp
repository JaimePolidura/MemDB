#include <string>
#include "PolynomialHashCreator.h"

PolynomialHashCreator::PolynomialHashCreator(int primeFactorCons): primeFactor {primeFactorCons} {}

int PolynomialHashCreator::create(const std::string &key) {
    int hashCode = 0;

    for(int i = 0; i < key.length(); i++)
        hashCode += key[i] * std::pow(this->primeFactor, i);

    return hashCode;
}