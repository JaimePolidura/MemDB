#pragma once

#include "shared.h"
#include "zlib.h"

#include "utils/std/Result.h"

class Compressor {
public:
    std::result<std::vector<uint8_t>, int> compressBytes(const std::vector<uint8_t>& uncompressed);

    std::result<std::vector<uint8_t>, int> uncompressBytes(const std::vector<uint8_t>& compressed, uint64_t originalSize);
};
