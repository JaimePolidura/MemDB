#include "utils/Utils.h"

bool Utils::tryOnce(std::function<void(void)> toTry) {
    try{
        toTry();
        return true;
    }catch (const std::exception& e) {
        return false;
    }
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

auto Utils::copyFromBuffer(const std::vector<uint8_t>& buffer, const uint8_t initialPosition, const uint8_t endPosition) -> uint8_t * {
    uint8_t * copyDest = new uint8_t[endPosition - initialPosition + 1];
    std::copy(buffer.begin() + initialPosition, buffer.begin() + endPosition + 1, copyDest);

    return copyDest;
}

std::vector<uint8_t> Utils::appendToBuffer(const uint8_t * toAppendPtr, const uint8_t totalToAppend, std::vector<uint8_t>& vectorOut) {
    for(auto i = 0; i < totalToAppend; i++) {
        vectorOut.push_back(* (toAppendPtr + i));
    }

    return vectorOut;
}