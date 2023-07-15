#include "utils/crypto/HashCalculator.h"

uint64_t HashCalculator::calculate(const std::string& input) {
    unsigned char md[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.length(), md);

    uint64_t result = 0;
    memcpy(&result, md, sizeof(result));

    return toBigEndian(result);
}

uint64_t HashCalculator::toBigEndian(uint64_t value) {
    uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
    uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));

    return (static_cast<uint64_t>(low_part) << 32) | high_part;
}

uint32_t HashCalculator::htonl(uint32_t hostlong) {
    uint32_t netlong = 0;
    uint8_t * ptr = reinterpret_cast<uint8_t*>(&hostlong);
    netlong |= static_cast<uint32_t>(ptr[0]) << 24;
    netlong |= static_cast<uint32_t>(ptr[1]) << 16;
    netlong |= static_cast<uint32_t>(ptr[2]) << 8;
    netlong |= static_cast<uint32_t>(ptr[3]);
    return netlong;
}