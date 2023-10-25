#include "OplogCompressor.h"

OplogCompressor::OplogCompressor(logger_t logger): logger(logger) {}

std::result<std::vector<uint8_t>, int> OplogCompressor::compressOplog(const std::vector<uint8_t>& uncompressed) {
    const uint8_t * uncompressedDataPtr = uncompressed.data();
    uLong uncompressedSize = uncompressed.size();

    uLong compressedSize = compressBound(uncompressedSize);
    std::vector<uint8_t> compressed(compressedSize);
    compressed.resize(compressedSize);

    int ret = compress(compressed.data(), &compressedSize, uncompressedDataPtr, uncompressedSize);

    if(ret == Z_OK){
        this->logger->debugInfo("Compressed oplog from {0} bytes to {1} bytes", uncompressed.size(), compressedSize);
        return std::result<std::vector<uint8_t>, int>::ok(compressed);
    } else {
        return std::result<std::vector<uint8_t>, int>::error(ret);
    }
}

std::result<std::vector<uint8_t>, int> OplogCompressor::uncompressOplog(const std::vector<uint8_t>& compressed, uint64_t originalSize) {
    const uint8_t * compressedDataPtr = compressed.data();
    uLong compressedDataSize = compressed.size();

    std::vector<uint8_t> decompressed(originalSize);
    uLong originalSizeULong = static_cast<uLong>(originalSize);

    int ret = uncompress(decompressed.data(), &originalSizeULong , compressedDataPtr, compressedDataSize);

    return ret == Z_OK ?
           std::result<std::vector<uint8_t>, int>::ok(decompressed) :
           std::result<std::vector<uint8_t>, int>::error(ret);
}