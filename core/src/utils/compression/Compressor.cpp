#include "Compressor.h"

std::result<std::vector<uint8_t>, int> Compressor::compressBytes(const std::vector<uint8_t>& uncompressed) {
    const uint8_t * uncompressedDataPtr = uncompressed.data();
    uLong uncompressedSize = uncompressed.size();

    uLong compressedSize = compressBound(uncompressedSize);
    std::vector<uint8_t> compressed(compressedSize);

    int ret = compress(compressed.data(), &compressedSize, uncompressedDataPtr, uncompressedSize);

    compressed.resize(compressedSize);

    return ret == Z_OK ?
        std::ok<std::vector<uint8_t>, int>(compressed) :
        std::error<std::vector<uint8_t>, int>(ret);
}

std::result<std::vector<uint8_t>, int> Compressor::uncompressBytes(const std::vector<uint8_t>& compressed, uint64_t originalSize) {
    const uint8_t * compressedDataPtr = compressed.data();
    uLong compressedDataSize = compressed.size();

    std::vector<uint8_t> decompressed(originalSize);
    uLong originalSizeULong = static_cast<uLong>(originalSize);

    int ret = uncompress(decompressed.data(), &originalSizeULong , compressedDataPtr, compressedDataSize);

    return ret == Z_OK ?
           std::ok<std::vector<uint8_t>, int>(decompressed) :
           std::error<std::vector<uint8_t>, int>(ret);
}