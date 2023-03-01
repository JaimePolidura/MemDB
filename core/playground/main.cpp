#include "utils/Utils.h"
#include <cstdint>
#include <cstdio>

int main() {
    auto copySource = std::vector<uint8_t>{0x00, 0x00, 0x01, 0xFF};
    auto copyResult = Utils::copyFromBuffer(copySource, 2, 3);
    for(int i = 0; i < 2; i++){
        auto val =  * (copyResult + i);
        auto wtf = 1;
    }


    printf("%i\n", 1);
}