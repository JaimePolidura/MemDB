#pragma once

#include "shared.h"
#include "zlib.h"

#include "logging/Logger.h"
#include "utils/std/Result.h"

class OplogCompressor {
private:
    logger_t logger;

public:
    OplogCompressor(logger_t logger);

    std::result<std::vector<uint8_t>, int> compressOplog(const std::vector<uint8_t>& uncompressed);

    std::result<std::vector<uint8_t>, int> uncompressOplog(const std::vector<uint8_t>& compressed, uint64_t originalSize);
};
