#include "utils/Utils.h"

bool Utils::tryOnce(std::function<void(void)> toTry) {
    try{
        toTry();
        return true;
    }catch (const std::exception& e) {
        return false;
    }
}

void Utils::printVectorHex(const std::vector<uint8_t>& toPrint) {
    for (const uint8_t byte : toPrint) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
}

void Utils::printVector(const std::vector<uint8_t>& toPrint) {
    std::cout << "[";

    for (size_t i = 0; i < toPrint.size(); ++i) {
        std::cout << static_cast<unsigned>(toPrint[i]);
        if (i != toPrint.size() - 1)
            std::cout << ", ";
    }
    std::cout << "]" << std::endl;
}

auto Utils::copyFromBuffer(const std::vector<uint8_t>& buffer, const uint64_t initialPosition, const uint64_t endPosition) -> uint8_t * {
    uint8_t * copyDest = new uint8_t[endPosition - initialPosition + 1];
    std::copy(buffer.begin() + initialPosition, buffer.begin() + endPosition + 1, copyDest);

    return copyDest;
}

std::vector<uint8_t> Utils::appendToBuffer(const uint8_t * toAppendPtr, const uint64_t totalToAppend, std::vector<uint8_t>& vectorOut) {
    for(auto i = 0; i < totalToAppend; i++) {
        vectorOut.push_back(* (toAppendPtr + i));
    }

    return vectorOut;
}

uint32_t Utils::crc(const std::vector<uint8_t>& values) {
    boost::crc_32_type crcCalculator;
    crcCalculator.process_bytes(values.data(), values.size());
    return crcCalculator.checksum();
}