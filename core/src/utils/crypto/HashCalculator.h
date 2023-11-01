#pragma once

#include "shared.h"

class HashCalculator {
public:
    static uint64_t calculateMD5(const std::string& input);

    static uint64_t calculatePolynomial();

private:
    static uint64_t toBigEndian(uint64_t value);

    static uint32_t htonl(uint32_t hostlong);
};