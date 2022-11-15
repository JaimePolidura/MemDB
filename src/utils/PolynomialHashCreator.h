#pragma once

#include "HashCreator.h"
#include "DBMap.h"
#include <string>
#include <tgmath.h>

class PolynomialHashCreator : public HashCreator<std::string> {
private:
    int primeFactor;

public:
    PolynomialHashCreator(int primeFactorCons);

    int create(const std::string& key);
};