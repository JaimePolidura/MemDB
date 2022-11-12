#include "HashCreator.h"

#include <string>
#include <tgmath.h>

class PolynomialHashCreator : public HashCreator<std::string> {
private:
    int primeFactor;

public:
    PolynomialHashCreator(int primceFactorCons): primeFactor{primceFactorCons} {};

    int create(const std::string& key) {
        int hashCode = 0;

        for(int i = 0; i < key.length(); i++)
            hashCode += key[i] * std::pow(this->primeFactor, i);

        return hashCode;
    }
};